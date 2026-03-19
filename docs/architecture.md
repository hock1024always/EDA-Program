# 架构设计文档

## 概述

EDA Integrated 采用分层架构，从底层数据模型到上层 GUI，每层均有明确的职责边界，层间通过头文件中定义的接口通信。

```
┌─────────────────────────────────────────────────────────────┐
│                     GUI 层 (Qt6)                             │
│   MainWindow  CircuitCanvas  PlacementView  AlgorithmPanel  │
└───────────────────────────┬─────────────────────────────────┘
                             │ 使用
┌───────────────────────────▼─────────────────────────────────┐
│                   应用层 (CLI / 示例)                        │
│            eda_cli (main_cli.cpp)  examples/                 │
└──────────┬─────────────────────┬───────────────┬────────────┘
           │                     │               │
┌──────────▼──────┐  ┌───────────▼──────┐  ┌────▼────────────┐
│   解析器层       │  │   布局算法层      │  │  可视化层       │
│  BookShelfParser│  │  InitialPlacer   │  │PlacementVisualiz│
│  VerilogParser  │  │  GlobalPlacer    │  │ CircuitRenderer │
└──────────┬──────┘  └───────────┬──────┘  └────┬────────────┘
           │                     │               │
┌──────────▼─────────────────────▼───────────────▼────────────┐
│                      核心数据层                               │
│        CircuitModel  Module  Net  Pin  Row  Point2D          │
└─────────────────────────────────────────────────────────────┘
```

---

## 各层职责

### 核心数据层 (`include/core/`, `src/core/`)

**核心类：**

```
CircuitModel          ← 顶层容器，持有整个电路设计
├─ Module             ← 单一模块/单元（标准单元、宏、端口）
│   └─ Pin            ← 模块上的连接引脚
└─ Net                ← 信号网络，关联多个 Pin
```

**设计原则：**
- 所有对象通过 `std::shared_ptr` 管理生命周期
- `CircuitModel` 通过 `module_map` / `net_map` 提供 O(1) 查找
- `Pin` 保存弱引用 (`weak_ptr`) 指向其父 `Module`，避免循环引用
- `Net::calcHPWL()` 直接从 `pin->position` 计算，布局后需先调用 `updateAbsolutePosition()`

**关键数据流：**

```
解析 → CircuitModel::addModule/addNet → 布局 → pin->updateAbsolutePosition() → 计算 HPWL/密度
```

---

### 解析器层 (`include/parser/`, `src/parser/`)

所有解析器均遵循统一接口：接受文件路径，填充 `CircuitModel`，返回 `bool`。

#### BookShelfParser

```
BookShelfParser::parse(aux_file, circuit)
    ├─ parseAuxFile()      → 读取文件列表
    ├─ parseNodesFile()    → 填充 Module（含 terminal 判断）
    ├─ parseNetsFile()     → 填充 Net + Pin，建立关联
    ├─ parseSclFile()      → 填充 Row，推导 die_area
    ├─ parsePlFile()       → 更新 Module::position（初始坐标）
    └─ parseWtsFile()      → 更新 Net::weight（可选）
```

**位置约定：** BookShelf 的 `.pl` 文件中坐标是模块中心，内部统一存储为左下角坐标，通过 `Module::setCenter()` 转换。

#### VerilogParser

```
VerilogLexer::nextToken()          → 词法分析
VerilogParser::parseModule()
    ├─ parseModuleHeader()         → 模块名 + 端口列表
    ├─ parsePortDeclarations()     → input / output 声明
    ├─ parseWireDeclarations()     → wire 声明
    ├─ parseInstances()            → 门级实例化提取
    └─ convertToCircuitModel()     → 生成 Module / Net / Pin
```

**局限：** 当前支持结构化门级 Verilog，不支持 RTL 行为描述或 `always` 块。

---

### 布局算法层 (`include/placer/`, `src/placer/`)

所有布局器继承 `PlacerBase`，实现 `run()` 和 `calculateOverflow()` 虚函数。

#### 接口设计

```cpp
class PlacerBase {
public:
    virtual bool run() = 0;
    virtual PlacerType getType() const = 0;
    virtual std::string getName() const = 0;

    void setProgressCallback(ProgressCallback cb);
    const PlacementStats& getStats() const;
    double calculateHPWL() const;
};
```

**统计结构 `PlacementStats`：**

```cpp
struct PlacementStats {
    double initial_hpwl, final_hpwl, hpwl_improvement;
    double initial_overflow, final_overflow;
    int iterations;
    double runtime_seconds;
};
```

#### InitialPlacer（Kraftwerk2A）

**算法流程：**

```
1. prepareData()
   └─ 统计可移动模块，建立 Module* → index 映射

2. buildLaplacianMatrix()
   ├─ 遍历所有 Net（degree ≥ 2）
   ├─ 权重 w = 1 / (degree - 1)     ← B2B 网模型
   ├─ 可移动-可移动对：加入拉普拉斯矩阵对角/非对角项
   └─ 可移动-固定对：加入 RHS 向量 b_x / b_y

3. initializePositions()
   └─ 所有可移动模块初始化到 die_area 中心

4. solveDirection(x, b_x)  +  solveDirection(y, b_y)
   └─ Eigen BiCGSTAB 求解稀疏线性方程组 A·x = b

5. updateModulePositions()  +  clipToCoreRegion()
   └─ 将解写回 Module::position，裁剪到合法范围
```

**数学背景：**

最小化二次线长目标函数：

```
minimize  Σ_net  w_net · Σ_{(i,j) ∈ net}  [(xi - xj)² + (yi - yj)²]
```

对 x、y 方向分别构造对称正定稀疏矩阵，BiCGSTAB 在约 100 次迭代内收敛。

#### GlobalPlacer（ePlace-MS 简化版）

**算法流程：**

```
1. initializePlacement()
   └─ 随机初始化可移动模块位置

2. initDensityGrid()
   └─ 创建 32×32 密度 bin 网格

3. initFillers()
   └─ 根据目标密度创建填充单元

4. 梯度下降主循环（最多 max_iterations 次）：
   a. updateDensity()                  ← 计算每个 bin 的密度
   b. computeWirelengthGradient()      ← HPWL 梯度（简化 WA 模型）
   c. computeDensityGradient()         ← 密度梯度（推开高密度区域）
   d. 合并梯度：∇f = ∇WL + λ · ∇Density
   e. computeStepSize()                ← 基于最大梯度幅值
   f. applyGradient()                  ← 移动模块，裁剪到合法范围

5. removeFillers()
   └─ 从模型中删除填充单元
```

**密度惩罚权重 λ** 随迭代次数线性增大：`λ = 0.1 × (1 + iter / 50)`

---

### 可视化层 (`include/visualizer/`, `src/visualizer/`)

#### PlacementVisualizer

负责将 `CircuitModel` 的布局状态渲染为 BMP 图像：

```
exportToBMP(filename, options)
    ├─ BMPWriter 初始化（1200×1200 像素）
    ├─ 填充白色背景
    ├─ drawDensity()     ← 可选：密度热力图（蓝→绿→黄→红）
    ├─ drawNets()        ← 可选：连线（灰色虚线）
    ├─ drawModules()     ← 模块矩形（颜色按类型区分）
    └─ 绘制 die_area 边界
```

**坐标变换：** 世界坐标 → 图像像素

```
scale = min(image_width / die_width, image_height / die_height) × margin
pixel_x = margin + (world_x - die_x_min) × scale
pixel_y = margin + (world_y - die_y_min) × scale
```

**BMP 格式：** 使用 24-bit BGR、无压缩，自包含不依赖图像库。

---

### GUI 层 (`include/gui/`, `src/gui/`)

#### 组件职责

| 组件 | 类名 | 职责 |
|------|------|------|
| 主窗口 | `MainWindow` | 菜单、工具栏、子组件协调、文件操作 |
| 逻辑图 | `CircuitCanvas` | 逻辑连接关系（QGraphicsView） |
| 布局图 | `PlacementView` | 物理位置显示、密度热力图 |
| 控制面板 | `AlgorithmPanel` | 参数设置、算法执行触发 |
| 进度框 | `ProgressDialog` | 长任务进度展示 |

#### 数据流动

```
用户操作（打开文件）
    → MainWindow::loadCircuit()
        → BookShelfParser / VerilogParser
        → CircuitModel 填充
    → CircuitCanvas::setCircuit()    ← 更新逻辑图
    → PlacementView::setCircuit()    ← 更新布局图

用户操作（运行布局）
    → AlgorithmPanel → signal: initialPlacementRequested()
    → MainWindow::runInitialPlacement()
        → InitialPlacer::run()
        → PlacementView::drawLayout()  ← 刷新布局图
```

#### 线程注意事项

当前版本布局算法在主线程执行。对于大规模设计（>10 万单元），建议使用 `QThread` + 信号槽将布局移到后台线程，通过 `ProgressCallback` 发送进度信号至主线程更新 UI。

---

## 编译目标依赖关系

```
eda_core (INTERFACE)          ← 仅头文件，供所有模块使用
    ↑
eda_parser (STATIC)           ← bookshelf_parser + verilog_parser
eda_placer (STATIC)           ← initial_placer + global_placer + placer_base
eda_visualizer (STATIC)       ← placement_visualizer + circuit_renderer
eda_utils (STATIC)            ← math_utils + file_utils + logger
    ↑
eda_cli (EXECUTABLE)          ← 链接上述所有 STATIC 库
eda_gui (EXECUTABLE)          ← 额外链接 Qt6::Core Qt6::Widgets Qt6::Gui
eda_tests (EXECUTABLE)        ← 链接 GTest + 各功能库
```

---

## 扩展方式

### 添加新的布局算法

1. 继承 `PlacerBase`，实现 `run()` 和 `getName()`
2. 在 `PlacerFactory::create()` 中注册新类型
3. 在 `AlgorithmPanel` 中添加对应的 UI 选项

```cpp
class MyPlacer : public PlacerBase {
public:
    explicit MyPlacer(CircuitModel& c) : PlacerBase(c) {}
    bool run() override { /* ... */ return true; }
    PlacerType getType() const override { return PlacerType::CUSTOM; }
    std::string getName() const override { return "My Placer"; }
};
```

### 添加新的解析格式

1. 在 `include/parser/` 创建头文件
2. 在 `src/parser/` 实现解析逻辑，统一签名：

```cpp
bool MyParser::parse(const std::string& filename, CircuitModel& circuit);
```

3. 在 `main_cli.cpp` 中增加格式检测分支
