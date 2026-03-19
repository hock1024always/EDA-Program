# 布局算法说明

本文档详细描述 EDA_Integrated 中实现的两种布局算法的数学原理、实现细节与参数调优建议。

---

## 目录

1. [布局问题概述](#布局问题概述)
2. [Kraftwerk2A 初始布局](#kraftwerk2a-初始布局)
3. [ePlace-MS 全局布局（简化版）](#eplace-ms-全局布局简化版)
4. [算法对比](#算法对比)
5. [参数调优指南](#参数调优指南)
6. [参考文献](#参考文献)

---

## 布局问题概述

### 问题定义

给定：
- 一组模块 $\mathcal{M} = \{m_1, m_2, \ldots, m_n\}$，每个模块有固定尺寸 $(w_i, h_i)$
- 一组网络 $\mathcal{N} = \{e_1, e_2, \ldots, e_k\}$，每条网连接若干模块的引脚
- 芯片区域（die area）和放置行（rows）

目标：确定每个可移动模块的坐标 $(x_i, y_i)$，使得：

$$\min \sum_{e \in \mathcal{N}} w_e \cdot \text{HPWL}(e)$$

约束：
- 所有模块必须位于 die area 内
- 可移动模块不能相互重叠
- 固定模块（terminal）位置不变

### 半周长线长（HPWL）

$$\text{HPWL}(e) = \left(\max_{i \in e} x_i - \min_{i \in e} x_i\right) + \left(\max_{i \in e} y_i - \min_{i \in e} y_i\right)$$

HPWL 是导线长度的一个良好近似，计算高效且与实际布线长度相关性强。

### 密度溢出（Density Overflow）

布局质量的另一个重要指标，衡量区域内模块堆叠程度：

$$\text{Overflow} = \frac{\sum_{b} \max(0, \rho_b - \rho_t) \cdot A_b}{\sum_{b} \rho_t \cdot A_b}$$

其中 $\rho_b$ 为 bin $b$ 的实际密度，$\rho_t$ 为目标密度，$A_b$ 为 bin 面积。

---

## Kraftwerk2A 初始布局

### 算法背景

Kraftwerk2A 是一种基于**二次规划（Quadratic Programming）**的解析式布局算法，通过将 HPWL 用平滑函数近似，将布局问题转化为线性方程组求解。

**原始论文**：  
Spindler et al., "Kraftwerk2 – A Fast Force-Directed Quadratic Placement Approach Using an Accurate Net Model," *IEEE TCAD*, 2008.

### 网模型：B2B（Bound-to-Bound）

对于包含 $k$ 个引脚的网络 $e$，B2B 模型将其分解为若干二引脚连接：

设 $p_{\max}^x = \arg\max_{i \in e} x_i$，$p_{\min}^x = \arg\min_{i \in e} x_i$，则：

$$W_e^x = \frac{w_e}{x_{\max} - x_{\min} + \epsilon} \cdot 2$$

每个非极值引脚 $i$ 与最近的极值引脚之间的连接权重为 $W_e^x$。

这种分解使得目标函数变为二次型：

$$f(x) = \frac{1}{2} x^T A x - b^T x$$

其中 $A$ 为加权拉普拉斯矩阵，$b$ 为来自固定模块（terminal）的常数项。

### 线性方程组求解

最优性条件 $\nabla f = 0$ 给出：

$$Ax = b_x, \quad Ay = b_y$$

本实现使用 **Eigen3 BiCGSTAB**（双共轭梯度稳定法）求解稀疏线性方程组：

```
A · x_solution = b_rhs
```

BiCGSTAB 适合非对称稀疏矩阵，收敛速度通常优于共轭梯度法（CG）。

### 实现细节

```
PrepareData()
  ├── 建立 module → 矩阵行索引映射（跳过固定模块）
  ├── 预分配 A (n_free × n_free)、b_x、b_y 向量
  └── 初始化 x、y 解向量为 die area 中心

BuildLaplacianMatrix()
  └── 对每条网 e：
      ├── 找到 x 方向的最大/最小引脚（B2B 极值引脚）
      ├── 计算连接权重 W = w_e / (span + ε)
      ├── 对每个非极值引脚 i：
      │   ├── A[i][i] += W
      │   ├── A[i][i_extreme] -= W（若 i_extreme 自由）
      │   └── b[i] += W * x_extreme（若 i_extreme 固定）
      └── 对极值引脚 i_max, i_min 之间：
          ├── A[i_max][i_max] += W
          ├── A[i_min][i_min] += W
          └── A[i_max][i_min] -= W（若均自由）

SolveDirection(A, b) → BiCGSTAB
  └── 迭代至收敛或达到 max_iterations

UpdateModulePositions()
  └── 将解向量写回各 Module::position

ClipToCoreRegion()
  └── 将超出 die area 的模块移至边界内
```

### 收敛特性

| 参数 | 说明 | 推荐值 |
|------|------|--------|
| `max_iterations` | BiCGSTAB 最大迭代次数 | 100–500 |
| `target_error` | BiCGSTAB 相对误差容限 | 1e-4–1e-6 |

初始布局通常**一次求解**即可，不需要迭代外层循环（与力导向方法不同）。

---

## ePlace-MS 全局布局（简化版）

### 算法背景

ePlace-MS 是一种基于**静电学类比**的解析式全局布局算法，专门设计用于处理混合尺寸（Mixed-Size）设计（既有标准单元，又有大型宏单元）。

**原始论文**：  
Lu et al., "ePlace-MS: Electrostatics-Based Placement for Mixed-Size Circuits," *IEEE TCAD*, 2015.

### 核心思想

将所有模块视为**带电粒子**，引入虚拟静电场：

1. **密度惩罚**：当某区域模块密度超过目标密度时，产生斥力，推动模块分散
2. **线长优化**：最小化 HPWL 的平滑近似，产生引力，使连接模块靠近
3. **梯度下降**：综合两种力的梯度，迭代更新模块位置

### 目标函数

$$\min_{x, y} \quad W_\text{smooth}(x, y) + \lambda \cdot D(x, y)$$

其中：
- $W_\text{smooth}$：HPWL 的可微近似（WA 模型）
- $D(x, y)$：密度惩罚函数
- $\lambda$：惩罚系数（随迭代逐渐增大）

### WA 线长平滑近似

对于网络 $e$，使用 Log-Sum-Exp（LSE）平滑：

$$W_\text{smooth}^x(e) = \gamma \left(\ln \sum_{i \in e} e^{x_i/\gamma} + \ln \sum_{i \in e} e^{-x_i/\gamma}\right)$$

梯度为：

$$\frac{\partial W_\text{smooth}^x}{\partial x_i} = \frac{e^{x_i/\gamma}}{\sum_{j} e^{x_j/\gamma}} - \frac{e^{-x_i/\gamma}}{\sum_{j} e^{-x_j/\gamma}}$$

本简化实现使用直接 HPWL 梯度（次梯度方法）代替 WA 模型，降低实现复杂度。

### 密度计算

将 die area 划分为 $B_x \times B_y$ 的均匀网格（bin），每个 bin 的密度：

$$\rho_b = \frac{\sum_{i \in \mathcal{M}} a_{ib}}{A_b}$$

其中 $a_{ib}$ 为模块 $i$ 与 bin $b$ 的**面积重叠**。

### 密度梯度

$$\frac{\partial D}{\partial x_i} = \sum_{b} \frac{\partial \rho_b}{\partial x_i} \cdot \max(0, \rho_b - \rho_t)$$

其中：

$$\frac{\partial \rho_b}{\partial x_i} = \frac{\partial a_{ib}}{\partial x_i} \cdot \frac{1}{A_b}$$

面积重叠对位置的偏导数可由几何关系解析得到。

### 填充单元（Filler）

为使密度计算更均匀，ePlace-MS 引入虚拟填充单元：

$$N_\text{filler} = \frac{(1 - \rho_t) \cdot A_\text{core} - A_\text{fixed}}{A_\text{filler\_cell}}$$

填充单元均匀分布在 die area 内，参与密度计算但不出现在最终布局结果中。

### 迭代流程

```
InitializePlacement()
  └── 在 die area 内随机/均匀撒点

InitDensityGrid()
  └── 创建 bins_x × bins_y 的密度网格

InitFillers()
  └── 计算并初始化填充单元

For iter = 1 to max_iterations:
  ├── UpdateDensity()
  │   └── 计算每个 bin 的实际密度 ρ_b
  │
  ├── ComputeWirelengthGradient(grad_x, grad_y)
  │   └── 对每条网用 B2B 模型计算 ∂HPWL/∂x_i
  │
  ├── ComputeDensityGradient(grad_x, grad_y)
  │   └── 对每个模块叠加密度斥力
  │
  ├── total_grad = wl_grad + λ * density_grad
  │
  ├── ApplyGradient(total_grad, step_size)
  │   └── x_i -= step_size * total_grad_x_i
  │
  ├── overflow = CalculateOverflow()
  │
  └── if overflow ≤ target_overflow: break

RemoveFillers()
ClipToCoreRegion()
```

### 自适应步长

步长 `step_size` 在迭代过程中自动衰减：

$$\text{step}_{t+1} = \text{step}_t \cdot \alpha, \quad \alpha \in [0.95, 0.99]$$

当 HPWL 出现震荡时，步长减半。

### 收敛特性

| 参数 | 说明 | 推荐值 |
|------|------|--------|
| `max_iterations` | 最大梯度下降迭代次数 | 100–500 |
| `target_overflow` | 收敛目标密度溢出率 | 0.05–0.20 |
| `target_density` | 目标放置密度 | 0.7–1.0 |
| `learning_rate` | 初始步长系数 | 0.05–0.20 |
| `bins_x/y` | 密度网格分辨率 | 32–128 |

---

## 算法对比

| 特性 | Kraftwerk2A（初始） | ePlace-MS（全局） |
|------|---------------------|-------------------|
| **数学模型** | 二次规划（QP） | 非线性梯度下降 |
| **线长近似** | B2B 二引脚分解 | WA（LSE 平滑）/ HPWL 次梯度 |
| **密度约束** | 无（后处理）| 静电场惩罚 |
| **求解器** | BiCGSTAB 线性求解 | 迭代梯度法 |
| **输出质量** | 良好线长，密度差 | 较好线长，密度均匀 |
| **适用场景** | 初始化、小规模设计 | 大规模、混合尺寸设计 |
| **运行时间** | 快（一次求解） | 慢（多次迭代） |
| **外部依赖** | Eigen3 | 无（仅标准库） |

### 推荐工作流

```
输入 BookShelf/Verilog
      ↓
Kraftwerk2A 初始布局     → 快速生成低重叠初始解
      ↓
ePlace-MS 全局布局       → 消除重叠，均匀铺展
      ↓
(未实现) 详细布局/合法化  → 对齐到放置行，消除小的重叠
      ↓
导出 .pl 文件
```

---

## 参数调优指南

### 初始布局（Kraftwerk2A）

**场景：BiCGSTAB 不收敛**
- 增加 `max_iterations`（如 500）
- 减小 `target_error`（如 1e-3，放宽容限）
- 检查网络是否存在孤立子图（无固定模块约束）

**场景：所有模块堆积在中心**
- 正常现象，Kraftwerk2A 不处理重叠
- 直接运行 ePlace-MS 全局布局

### 全局布局（ePlace-MS）

**场景：HPWL 不下降（震荡）**
- 减小 `learning_rate`（如 0.05）
- 增大 `bins_x/y`（提高密度分辨率）

**场景：收敛过慢**
- 增大 `learning_rate`（如 0.2）
- 减小 `bins_x/y`（如 32）加快密度计算
- 降低 `target_overflow`（如 0.15，放宽收敛条件）

**场景：最终仍有大量重叠**
- 检查设计利用率：`circuit.calcUtilization()`
- 若利用率 > 0.85，设计过于拥挤，重叠难以完全消除
- 尝试降低 `target_density`（如 0.8）允许更宽松的布局

**基准测试参考（ISPD 2006）**

| 测试集 | 模块数 | 推荐 bins | 推荐迭代 |
|--------|--------|-----------|----------|
| adaptec1 | 211,447 | 64×64 | 200 |
| adaptec2 | 255,023 | 64×64 | 200 |
| bigblue1 | 278,164 | 64×64 | 300 |
| bigblue3 | 1,096,812 | 128×128 | 500 |

---

## 参考文献

1. **Kraftwerk2**:  
   P. Spindler, U. Schlichtmann, F. M. Johannes, "Kraftwerk2 – A Fast Force-Directed Quadratic Placement Approach Using an Accurate Net Model," *IEEE Transactions on Computer-Aided Design of Integrated Circuits and Systems*, vol. 27, no. 8, 2008.

2. **ePlace**:  
   J. Lu, P. Chen, C.-C. Chang, L. Sha, D. J.-H. Huang, C.-C. Teng, C.-K. Cheng, "ePlace: Electrostatics-Based Placement Using Fast Fourier Transform and Nesterov's Method," *ACM Transactions on Design Automation of Electronic Systems*, vol. 20, no. 2, 2015.

3. **ePlace-MS**:  
   J. Lu, H. Zhuang, P. Chen, H. Chang, C.-C. Chang, Y.-C. Wong, L. Sha, D. Huang, Y. Luo, C.-C. Teng, C.-K. Cheng, "ePlace-MS: Electrostatics-Based Placement for Mixed-Size Circuits," *IEEE Transactions on Computer-Aided Design of Integrated Circuits and Systems*, vol. 34, no. 5, 2015.

4. **ISPD 2006 Placement Contest**:  
   M. C. Kim, D. J. Lee, I. L. Markov, "SimPL: An Effective Placement Algorithm," *IEEE TCAD*, 2012.

5. **HPWL 作为布线代价近似**:  
   C. J. Alpert, D. P. Mehta, S. S. Sapatnekar, *Handbook of Algorithms for Physical Design Automation*, CRC Press, 2009.
