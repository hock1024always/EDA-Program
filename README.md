# EDA Integrated

> 一个整合电路编辑、Verilog 解析、VLSI 布局优化的开源 EDA 工具

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.16+-green.svg)](https://cmake.org/)
[![Qt6](https://img.shields.io/badge/Qt-6.x-brightgreen.svg)](https://www.qt.io/)

---

## 项目简介

**EDA Integrated** 是一个面向教学场景的开源电子设计自动化（EDA）工具集，将三个独立的综合设计实验整合为一套完整的电路设计与自动布局流程：

| 阶段 | 功能 | 来源 |
|------|------|------|
| EDA01 | 交互式电路原理图编辑 | 逻辑门绘制与仿真 |
| EDA02 | Verilog 网表解析与可视化 | 编译器前端 + Graphviz |
| EDA03 | VLSI 自动布局与优化 | Kraftwerk2A + ePlace-MS |

该工具支持从 Verilog 文件或 BookShelf 格式导入电路，经过初始布局（二次规划）和全局布局（静电学模型）优化后，将结果以可视化形式展示，并导出为标准 `.pl` 格式。

---

## 功能特性

### 解析器
- **BookShelf 格式**：完整支持 `.aux` / `.nodes` / `.nets` / `.scl` / `.pl` / `.wts`
- **Verilog 网表**：支持结构化 Verilog，包含词法分析、语法分析、实例化提取

### 布局算法
- **Kraftwerk2A 初始布局**：基于 B2B 网模型的二次规划，使用 BiCGSTAB 迭代求解器
- **ePlace-MS 全局布局**：基于密度惩罚的梯度下降，支持 filler cells

### 可视化
- BMP 格式布局图导出
- 密度热力图
- Qt6 双视图图形界面（电路图 + 物理布局）

### 命令行工具
- 完整的 `parse` / `place` / `visualize` 三个子命令
- 支持自动格式检测

---

## 快速开始

### 依赖要求

| 依赖 | 版本要求 | 说明 |
|------|----------|------|
| GCC / Clang | ≥ 9 / ≥ 10 | 支持 C++17 |
| CMake | ≥ 3.16 | 构建系统 |
| Eigen3 | ≥ 3.3 | 稀疏矩阵与线性求解器 |
| Qt6 | ≥ 6.0 | GUI 界面（可选） |
| Google Test | any | 单元测试（可选） |

### 安装依赖

**Ubuntu / Debian:**
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake
sudo apt-get install -y libeigen3-dev
sudo apt-get install -y qt6-base-dev           # 可选：GUI
sudo apt-get install -y libgtest-dev           # 可选：测试
```

**macOS (Homebrew):**
```bash
brew install cmake eigen qt@6 googletest
export PATH="/opt/homebrew/opt/qt@6/bin:$PATH"
```

**Windows (vcpkg):**
```powershell
vcpkg install eigen3 qt6-base gtest
```

### 编译

```bash
# 克隆项目
git clone https://github.com/yourname/EDA_Integrated.git
cd EDA_Integrated

# 创建构建目录
mkdir build && cd build

# 配置（包含 GUI）
cmake .. -DBUILD_GUI=ON -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release

# 配置（仅命令行，无 Qt 依赖）
cmake .. -DBUILD_GUI=OFF -DBUILD_TESTS=ON

# 编译
make -j$(nproc)
```

### 验证安装

```bash
# 解析一个 BookShelf 设计并显示统计信息
./eda_cli parse /path/to/design.aux

# 运行布局并导出结果
./eda_cli place --visualize design.aux

# 启动 GUI
./eda_gui design.aux
```

---

## 使用说明

### 命令行工具 `eda_cli`

```
Usage: eda_cli <command> [options] <input_file>

Commands:
  parse       解析并显示电路信息
  place       运行布局算法
  visualize   导出可视化图像

Options:
  -h, --help              显示帮助
  -f, --format <fmt>      指定格式：bookshelf | verilog
  -o, --output <file>     输出文件路径
  -a, --algorithm <alg>   布局算法：initial | global | both（默认 both）
  -v, --verbose           详细输出
  --visualize             生成 BMP 可视化图像
```

**示例：**

```bash
# 解析 BookShelf 设计
./eda_cli parse adaptec1.aux

# 解析 Verilog 网表（详细输出）
./eda_cli parse -f verilog -v design.v

# 运行完整布局流程并生成图像
./eda_cli place --visualize -o result.pl design.aux

# 仅运行初始布局
./eda_cli place -a initial -o init.pl design.aux

# 导出布局可视化
./eda_cli visualize -o layout.bmp design.aux
```

### GUI 应用 `eda_gui`

启动后界面分为三个区域：

```
┌─────────────────────────────────────┬──────────────────┐
│  菜单栏  工具栏                      │                  │
├──────────────────┬──────────────────┤  算法控制面板    │
│                  │                  │                  │
│  电路原理图视图  │  物理布局视图    │  参数设置        │
│  (CircuitCanvas) │  (PlacementView) │  执行按钮        │
│                  │                  │  进度显示        │
├──────────────────┴──────────────────┴──────────────────┤
│  状态栏                                                 │
└─────────────────────────────────────────────────────────┘
```

**操作方式：**
- `Ctrl + 滚轮`：缩放视图
- 左键拖拽：平移视图
- `Ctrl + 0`：适应窗口
- File → Open：打开 `.aux` 或 `.v` 文件
- Placement → Full Flow：运行完整布局

---

## 项目结构

```
EDA_Integrated/
├── CMakeLists.txt              # 主构建配置
├── README.md                   # 本文件
│
├── include/                    # 公共头文件
│   ├── core/
│   │   └── circuit_model.h     # 核心数据模型（Module / Net / Pin / CircuitModel）
│   ├── parser/
│   │   ├── bookshelf_parser.h  # BookShelf 格式解析器
│   │   └── verilog_parser.h    # Verilog 网表解析器
│   ├── placer/
│   │   ├── placer_base.h       # 布局器基类与统一接口
│   │   ├── initial_placer.h    # Kraftwerk2A 初始布局
│   │   └── global_placer.h     # ePlace-MS 全局布局
│   ├── visualizer/
│   │   └── placement_visualizer.h  # BMP 图像导出
│   └── gui/
│       ├── main_window.h       # 主窗口
│       ├── circuit_canvas.h    # 电路原理图画布
│       ├── placement_view.h    # 物理布局视图
│       ├── algorithm_panel.h   # 算法控制面板
│       └── progress_dialog.h   # 进度对话框
│
├── src/                        # 源文件（与 include 对应）
│   ├── core/circuit_model.cpp
│   ├── parser/
│   │   ├── bookshelf_parser.cpp
│   │   ├── verilog_parser.cpp
│   │   └── parser_utils.cpp
│   ├── placer/
│   │   ├── placer_base.cpp
│   │   ├── initial_placer.cpp
│   │   └── global_placer.cpp
│   ├── visualizer/placement_visualizer.cpp
│   ├── gui/
│   │   ├── main.cpp            # GUI 应用入口
│   │   ├── main_window.cpp
│   │   ├── circuit_canvas.cpp
│   │   ├── placement_view.cpp
│   │   ├── algorithm_panel.cpp
│   │   └── progress_dialog.cpp
│   └── main_cli.cpp            # CLI 工具入口
│
├── tests/                      # 单元测试
│   ├── test_circuit_model.cpp
│   ├── test_bookshelf_parser.cpp
│   ├── test_verilog_parser.cpp
│   ├── test_placement.cpp
│   └── test_gui.cpp
│
├── examples/                   # 示例程序
│   ├── example_circuit.cpp     # 手动构造电路
│   ├── example_bookshelf.cpp   # 解析 BookShelf
│   ├── example_verilog.cpp     # 解析 Verilog
│   └── example_placement.cpp   # 完整布局流程
│
└── docs/                       # 文档
    ├── architecture.md         # 架构设计文档
    ├── algorithms.md           # 算法说明文档
    ├── api_reference.md        # API 参考文档
    ├── gui_manual.md           # GUI 使用手册
    └── contributing.md         # 贡献指南
```

---

## 数据模型

核心数据类 `CircuitModel` 是所有模块的统一表示，对应如下层次：

```
CircuitModel
├── name                   设计名称
├── die_area               芯片区域 (x_min, y_min, x_max, y_max)
├── modules: [Module]
│   ├── name               模块名
│   ├── type               STANDARD_CELL | MACRO | TERMINAL | FILLER
│   ├── position           左下角坐标
│   ├── width, height      几何尺寸
│   ├── is_fixed           是否固定
│   └── pins: [Pin]
│       ├── name           引脚名
│       ├── direction      INPUT | OUTPUT | INOUT
│       ├── offset         相对模块原点偏移
│       └── position       绝对位置（布局后更新）
├── nets: [Net]
│   ├── name               网络名
│   ├── pins: [Pin*]       连接的引脚列表
│   ├── weight             权重（来自 .wts）
│   └── calcHPWL()         计算半周长线长
└── rows: [Row]            标准单元放置行
```

---

## 支持格式

### 输入格式

| 格式 | 文件后缀 | 说明 |
|------|----------|------|
| BookShelf | `.aux` + `.nodes` + `.nets` + `.scl` + `.pl` | ISPD 竞赛标准格式 |
| Verilog | `.v` / `.sv` | 结构化网表（门级） |

### 输出格式

| 格式 | 文件后缀 | 说明 |
|------|----------|------|
| BookShelf Placement | `.pl` | UCLA pl 1.0 格式 |
| BMP Image | `.bmp` | 布局可视化图像 |

---

## 开发计划

- [x] Phase 1：统一数据模型、解析器、测试框架
- [x] Phase 2：Kraftwerk2A、ePlace-MS、BMP 可视化
- [x] Phase 3：Qt6 GUI 界面
- [ ] Phase 4（规划中）：
  - [ ] 多线程布局执行（后台线程 + 进度回调）
  - [ ] 布局过程逐帧动画
  - [ ] 交互式模块拖拽编辑
  - [ ] 详细设计规则检查（DRC）
  - [ ] 导出 SVG / PNG 格式

---

## 贡献

欢迎提交 Issue 和 Pull Request！请阅读 [CONTRIBUTING.md](docs/contributing.md) 了解贡献规范。

---

## 许可证

本项目遵循 [MIT License](LICENSE) 开源协议。

---

## 参考文献

1. Kraftwerk2 — A Fast Force-Directed Quadratic Placement Approach Using an Accurate Net Model. *IEEE TCAD 2008*
2. ePlace: Electrostatics-Based Placement Using Fast Fourier Transform and Nesterov's Method. *ACM TODAES 2015*
3. ISPD 2006 Placement Contest Benchmarks — BookShelf Format Specification
