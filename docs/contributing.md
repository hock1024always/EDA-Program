# 开发者指南

欢迎参与 EDA_Integrated 项目！本文档说明如何搭建开发环境、遵循代码规范、提交 Pull Request 以及扩展系统功能。

---

## 目录

1. [开发环境搭建](#开发环境搭建)
2. [项目结构](#项目结构)
3. [代码规范](#代码规范)
4. [构建与测试](#构建与测试)
5. [提交规范](#提交规范)
6. [扩展指南](#扩展指南)
   - [添加新的解析器](#添加新的解析器)
   - [添加新的布局算法](#添加新的布局算法)
   - [添加新的可视化输出格式](#添加新的可视化输出格式)
7. [常见问题](#常见问题)

---

## 开发环境搭建

### 系统依赖

| 依赖 | 最低版本 | 说明 |
|------|----------|------|
| CMake | 3.16 | 构建系统 |
| GCC / Clang | GCC 9 / Clang 10 | 需要 C++17 支持 |
| Eigen3 | 3.3 | 稀疏矩阵和线性求解器 |
| Qt6 | 6.2 | GUI 框架（可选，仅构建 GUI 时需要） |
| Google Test | 1.10 | 单元测试框架（可选） |

### Ubuntu 22.04

```bash
sudo apt update
sudo apt install -y \
    cmake build-essential \
    libeigen3-dev \
    qt6-base-dev qt6-tools-dev \
    libgtest-dev
```

### macOS（Homebrew）

```bash
brew install cmake eigen qt@6 googletest
export CMAKE_PREFIX_PATH=$(brew --prefix qt@6):$CMAKE_PREFIX_PATH
```

### 克隆并初始构建

```bash
git clone https://github.com/your-org/EDA_Integrated.git
cd EDA_Integrated
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . -j$(nproc)
```

### 构建选项

| CMake 选项 | 默认 | 说明 |
|-----------|------|------|
| `EDA_BUILD_GUI` | ON | 构建 Qt6 GUI |
| `EDA_BUILD_TESTS` | ON | 构建 Google Test 测试 |
| `EDA_BUILD_EXAMPLES` | ON | 构建示例程序 |
| `CMAKE_BUILD_TYPE` | Release | Debug / Release / RelWithDebInfo |

仅构建核心库和 CLI（无 Qt6）：

```bash
cmake .. -DEDA_BUILD_GUI=OFF -DEDA_BUILD_TESTS=OFF
```

---

## 项目结构

```
EDA_Integrated/
├── include/               # 公共头文件（对外接口）
│   ├── core/              # 数据模型
│   ├── parser/            # 解析器接口
│   ├── placer/            # 布局算法接口
│   ├── visualizer/        # 可视化接口
│   ├── gui/               # Qt6 GUI 组件接口
│   └── utils/             # 工具类接口
├── src/                   # 实现文件
│   ├── core/
│   ├── parser/
│   ├── placer/
│   ├── visualizer/
│   ├── gui/
│   ├── utils/
│   ├── main_cli.cpp       # CLI 入口
│   └── main_gui.cpp       # GUI 入口
├── tests/                 # 单元测试
├── examples/              # 示例程序
├── docs/                  # 文档
└── CMakeLists.txt
```

**核心原则：头文件即接口。**  
`include/` 下的头文件是稳定的公共 API，`src/` 下的 `.cpp` 是私有实现，可以自由修改。

---

## 代码规范

### 命名约定

| 元素 | 规范 | 示例 |
|------|------|------|
| 类名 | `PascalCase` | `CircuitModel`, `BookShelfParser` |
| 函数/方法 | `camelCase` | `addModule()`, `calcHPWL()` |
| 私有成员变量 | `snake_case_` 尾部加 `_` | `die_area_`, `max_iter_` |
| 公有成员变量 | `snake_case` | `width`, `height`, `name` |
| 枚举值 | `UPPER_SNAKE_CASE` | `ModuleType::STANDARD_CELL` |
| 常量 | `kCamelCase` 或 `UPPER_SNAKE_CASE` | `kMaxIter`, `DEFAULT_BINS` |
| 文件名 | `snake_case` | `circuit_model.h`, `bookshelf_parser.cpp` |

### 文件组织

每个类对应一对 `.h` / `.cpp` 文件：

```cpp
// include/parser/my_parser.h
#pragma once
#include <string>
#include "core/circuit_model.h"

namespace eda {

class MyParser {
public:
    bool parse(const std::string& filename, CircuitModel& circuit);
    std::string getLastError() const { return last_error_; }

private:
    std::string last_error_;
};

} // namespace eda
```

```cpp
// src/parser/my_parser.cpp
#include "parser/my_parser.h"
#include <fstream>

namespace eda {

bool MyParser::parse(const std::string& filename, CircuitModel& circuit) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        last_error_ = "Cannot open file: " + filename;
        return false;
    }
    // ...
    return true;
}

} // namespace eda
```

### 错误处理

- **不抛出异常**：所有函数通过返回 `bool` 或 `std::optional` 报告错误
- **错误信息**：通过 `getLastError()` 提供给调用方
- **日志**：使用 `std::cerr` 输出警告，不使用 `std::cout`（保留给用户输出）

```cpp
// 好的做法
bool MyParser::parse(...) {
    if (!file.is_open()) {
        last_error_ = "Cannot open: " + filename;
        return false;
    }
    return true;
}

// 避免
bool MyParser::parse(...) {
    if (!file.is_open()) throw std::runtime_error("...");  // 不推荐
}
```

### 内存管理

- 优先使用智能指针：`std::shared_ptr`（`ModulePtr`, `NetPtr`）
- `CircuitModel` 拥有所有 `Module` 和 `Net` 的所有权
- GUI 组件遵循 Qt 对象树的生命周期管理

### 头文件保护

一律使用 `#pragma once`，不使用传统的 `#ifndef` 宏。

---

## 构建与测试

### 运行所有测试

```bash
cd build
ctest --output-on-failure
```

### 运行单个测试套件

```bash
./tests/test_circuit_model
./tests/test_bookshelf_parser
./tests/test_placer
```

### 添加新测试

在 `tests/` 目录下创建 `test_xxx.cpp`，并在 `tests/CMakeLists.txt` 中注册：

```cmake
add_executable(test_my_parser test_my_parser.cpp)
target_link_libraries(test_my_parser
    PRIVATE eda_parser eda_core GTest::gtest_main)
add_test(NAME MyParserTest COMMAND test_my_parser)
```

测试示例：

```cpp
#include <gtest/gtest.h>
#include "parser/my_parser.h"
#include "core/circuit_model.h"

using namespace eda;

TEST(MyParserTest, ParseValidFile) {
    MyParser parser;
    CircuitModel circuit;
    ASSERT_TRUE(parser.parse("testdata/simple.v", circuit));
    EXPECT_EQ(circuit.getNumModules(), 5u);
    EXPECT_EQ(circuit.getNumNets(), 4u);
}

TEST(MyParserTest, HandleMissingFile) {
    MyParser parser;
    CircuitModel circuit;
    EXPECT_FALSE(parser.parse("nonexistent.v", circuit));
    EXPECT_FALSE(parser.getLastError().empty());
}
```

---

## 提交规范

### Commit Message 格式

```
<type>(<scope>): <subject>

[可选正文，解释"为什么"而非"做了什么"]
```

**type 类型：**

| type | 说明 |
|------|------|
| `feat` | 新功能 |
| `fix` | Bug 修复 |
| `refactor` | 代码重构（不改变行为） |
| `perf` | 性能优化 |
| `test` | 添加或修改测试 |
| `docs` | 文档更新 |
| `build` | 构建系统/依赖更新 |
| `chore` | 其他维护性工作 |

**示例：**

```
feat(parser): add LEF format support for cell dimensions

Previously all cells used simplified hardcoded dimensions.
This change reads actual cell sizes from .lef files, improving
placement accuracy for real designs.
```

```
fix(placer): prevent division by zero in B2B net model

When a net has all pins at the same X coordinate, span=0
caused NaN in the weight calculation. Added epsilon clamp.
```

### Pull Request 流程

1. Fork 仓库，从 `main` 创建特性分支：
   ```bash
   git checkout -b feat/lef-parser
   ```
2. 编写代码，添加对应测试
3. 确保所有测试通过：`ctest --output-on-failure`
4. 提交 PR，描述中填写：
   - 解决了什么问题
   - 主要改动点
   - 如何测试

---

## 扩展指南

### 添加新的解析器

**场景**：支持 DEF（Design Exchange Format）文件。

**步骤 1**：创建头文件 `include/parser/def_parser.h`

```cpp
#pragma once
#include <string>
#include "core/circuit_model.h"

namespace eda {

class DefParser {
public:
    bool parse(const std::string& filename, CircuitModel& circuit);
    std::string getLastError() const { return last_error_; }

private:
    std::string last_error_;
    CircuitModel* circuit_ = nullptr;

    bool parseComponents(std::ifstream& file);
    bool parseNets(std::ifstream& file);
};

} // namespace eda
```

**步骤 2**：实现 `src/parser/def_parser.cpp`

**步骤 3**：将实现文件加入 `src/parser/CMakeLists.txt`（或主 CMakeLists.txt）：

```cmake
target_sources(eda_parser PRIVATE
    src/parser/bookshelf_parser.cpp
    src/parser/verilog_parser.cpp
    src/parser/def_parser.cpp        # 新增
)
```

**步骤 4**：在 `src/main_cli.cpp` 的 `cmdParse` 函数中添加 `.def` 的分支判断。

**步骤 5**：添加测试文件 `tests/test_def_parser.cpp`。

---

### 添加新的布局算法

**场景**：实现 SimPL 详细布局算法。

**步骤 1**：创建头文件 `include/placer/simpl_placer.h`，继承 `PlacerBase`：

```cpp
#pragma once
#include "placer/placer_base.h"

namespace eda {

class SimplPlacer : public PlacerBase {
public:
    explicit SimplPlacer(CircuitModel& circuit);

    bool run() override;
    PlacerType getType() const override { return PlacerType::CUSTOM; }
    std::string getName() const override { return "SimPL"; }
    double calculateOverflow() const override;

    void setTargetOverflow(double v) { target_overflow_ = v; }

private:
    double target_overflow_ = 0.1;
    // 内部数据结构...

    void initialSpread();
    void legalizeRows();
    void detailedPlace();
};

} // namespace eda
```

**步骤 2**：实现 `src/placer/simpl_placer.cpp`，在 `run()` 中调用内部步骤并更新 `stats_`。

**步骤 3**：在 `CMakeLists.txt` 的 `eda_placer` 库中添加新源文件。

**步骤 4**：在 GUI `AlgorithmPanel` 中添加新按钮，在 `MainWindow` 中添加对应 slot。

---

### 添加新的可视化输出格式

**场景**：导出 SVG 格式。

**步骤 1**：在 `include/visualizer/placement_visualizer.h` 中添加方法声明：

```cpp
bool exportToSVG(const std::string& filename,
                 const VisualizerOptions& options = VisualizerOptions());
```

**步骤 2**：在 `src/visualizer/placement_visualizer.cpp` 中实现：

```cpp
bool PlacementVisualizer::exportToSVG(const std::string& filename,
                                       const VisualizerOptions& options) {
    std::ofstream file(filename);
    if (!file.is_open()) return false;

    double scale_x = options.image_width  / circuit_.die_area.width();
    double scale_y = options.image_height / circuit_.die_area.height();

    // SVG 头部
    file << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
         << "width=\"" << options.image_width << "\" "
         << "height=\"" << options.image_height << "\">\n";

    // 绘制每个模块
    for (const auto& mod : circuit_.modules) {
        if (mod->type == ModuleType::FILLER) continue;
        double rx = (mod->position.x - circuit_.die_area.x_min) * scale_x;
        double ry = (mod->position.y - circuit_.die_area.y_min) * scale_y;
        double rw = mod->width  * scale_x;
        double rh = mod->height * scale_y;
        std::string color = (mod->is_fixed) ? "blue" : "red";
        file << "  <rect x=\"" << rx << "\" y=\"" << ry
             << "\" width=\"" << rw << "\" height=\"" << rh
             << "\" fill=\"" << color << "\" opacity=\"0.6\"/>\n";
    }

    file << "</svg>\n";
    return true;
}
```

**步骤 3**：在 CLI 的 `cmdVisualize` 和 GUI 的导出菜单中添加 SVG 选项。

---

## 常见问题

**Q: 编译时找不到 Eigen3？**

```bash
# Ubuntu
sudo apt install libeigen3-dev
# 或手动指定路径
cmake .. -DEigen3_DIR=/path/to/eigen3/cmake
```

**Q: Qt6 找不到？**

```bash
# 确保 Qt6 安装后设置 CMAKE_PREFIX_PATH
export CMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake/Qt6
cmake ..
```

**Q: 测试数据文件在哪里？**

单元测试使用内联生成的测试数据（字符串解析），不依赖外部文件。  
集成测试所需的 BookShelf 数据集可从 [ISPD 2006 Contest](http://www.ispd.cc/contests/06/) 下载。

**Q: 如何在不安装 Qt6 的环境中仅构建 CLI？**

```bash
cmake .. -DEDA_BUILD_GUI=OFF
cmake --build . --target eda_cli
```

**Q: 如何为新算法添加性能基准测试？**

在 `examples/` 下创建 `benchmark_xxx.cpp`，使用 `<chrono>` 计时，输出 HPWL 和 overflow 指标，与已有算法结果对比。
