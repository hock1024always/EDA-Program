# API 参考文档

## 命名空间

所有公共接口均位于 `eda` 命名空间下。

---

## 核心数据模型

### `CircuitModel`

电路设计的顶层容器。

```cpp
#include "circuit_model.h"
```

#### 构造

```cpp
CircuitModel();
explicit CircuitModel(const std::string& name);
```

#### 模块管理

```cpp
void addModule(ModulePtr module);
ModulePtr getModule(const std::string& name) const;
void removeModule(const std::string& name);
```

#### 网络管理

```cpp
void addNet(NetPtr net);
NetPtr getNet(const std::string& name) const;
void removeNet(const std::string& name);
```

#### 统计查询

```cpp
size_t getNumModules() const;
size_t getNumNets() const;
size_t getNumPins() const;
size_t getNumMovableModules() const;
size_t getNumFixedModules() const;
```

#### 分析方法

```cpp
// 计算总半周长线长（HPWL），布局后调用
double calcTotalHPWL() const;

// 计算可移动单元面积占行总面积的比例
double calcUtilization() const;

// 生成密度网格，返回 [bins_x][bins_y] 的二维数组
std::vector<std::vector<double>> calcDensityMap(
    int num_bins_x, int num_bins_y) const;
```

#### 导入/导出

```cpp
// 导出为 BookShelf .pl 格式
bool exportToPl(const std::string& filename) const;

// 清空所有数据
void clear();

// 检查数据一致性（名称唯一性、尺寸合法性、die area）
bool validate() const;
```

#### 成员变量

```cpp
std::string name;                        // 设计名称
DieArea die_area;                        // 芯片区域
std::vector<ModulePtr> modules;          // 所有模块
std::vector<NetPtr> nets;                // 所有网络
std::vector<Row> rows;                   // 放置行
double site_width = 0.0;                 // 站点宽度
double row_height = 0.0;                 // 行高度
```

---

### `Module`

```cpp
using ModulePtr = std::shared_ptr<Module>;
```

#### 构造

```cpp
explicit Module(const std::string& name);
```

#### 引脚管理

```cpp
void addPin(PinPtr pin);
PinPtr getPin(const std::string& pin_name) const;
```

#### 几何方法

```cpp
Rectangle getBBox() const;           // 返回 [position.x, position.y, x+w, y+h]
Point2D getCenter() const;
void setCenter(const Point2D& center);
bool overlaps(const Module& other) const;
```

#### 成员变量

```cpp
std::string name;
ModuleType type;          // STANDARD_CELL | MACRO | TERMINAL | FILLER | UNKNOWN
double width, height;
Point2D position;         // 左下角坐标
bool is_fixed;
bool is_placed;
std::string cell_type;    // 单元类型名（如 "NAND2X1"）
std::vector<PinPtr> pins;
```

---

### `Net`

```cpp
using NetPtr = std::shared_ptr<Net>;
```

> `Net` 继承 `std::enable_shared_from_this<Net>`，用于在 `addPin` 中自动绑定。

#### 引脚管理

```cpp
void addPin(PinPtr pin);
void removePin(PinPtr pin);
PinPtr getDriver() const;                       // 返回第一个 OUTPUT 引脚
std::vector<PinPtr> getLoads() const;           // 返回所有 INPUT 引脚
```

#### 分析方法

```cpp
double calcHPWL() const;          // 从 pin->position 计算半周长线长
void updateBoundingBox();         // 更新缓存的 bbox
```

#### 成员变量

```cpp
std::string name;
std::vector<PinPtr> pins;
double weight = 1.0;
Rectangle bbox;                   // 缓存的包围盒（调用 updateBoundingBox 后有效）
```

---

### `Pin`

```cpp
using PinPtr = std::shared_ptr<Pin>;
```

#### 方法

```cpp
void updateAbsolutePosition();    // position = parent_module->position + offset
```

#### 成员变量

```cpp
std::string name;
PinDirection direction;           // INPUT | OUTPUT | INOUT | UNKNOWN
Point2D offset;                   // 相对模块原点的偏移
Point2D position;                 // 绝对坐标（布局后调用 updateAbsolutePosition）
std::shared_ptr<Module> parent_module;  // 持有父模块（strong ref）
std::shared_ptr<Net> net;               // 所在网络
```

---

### 辅助结构体

#### `Point2D`

```cpp
struct Point2D {
    double x, y;
    Point2D operator+(const Point2D&) const;
    Point2D operator-(const Point2D&) const;
};
```

#### `Rectangle`

```cpp
struct Rectangle {
    double x_min, y_min, x_max, y_max;
    double width() const;
    double height() const;
    double area() const;
    Point2D center() const;
    bool contains(const Point2D&) const;
};
```

#### `DieArea`

```cpp
struct DieArea {
    double x_min, y_min, x_max, y_max;
    double width() const;
    double height() const;
    double area() const;
    bool contains(const Point2D&) const;
    Rectangle toRectangle() const;
};
```

#### `Row`

```cpp
struct Row {
    double y_coord;        // 行的 Y 坐标（底边）
    double height;         // 行高
    double x_min, x_max;  // 行的 X 范围
    bool is_horizontal;
    double width() const;
};
```

---

## 解析器

### `BookShelfParser`

```cpp
#include "bookshelf_parser.h"
```

```cpp
BookShelfParser parser;

// 从 .aux 文件解析完整设计
bool success = parser.parse("/path/to/design.aux", circuit);

if (!success) {
    std::cerr << parser.getLastError() << "\n";
}
```

**解析后 `circuit` 包含：**
- 所有模块（可移动单元 + terminal）
- 所有网络和引脚
- 放置行（来自 `.scl`）
- 初始位置（来自 `.pl`）
- die_area（从行信息推导）

---

### `VerilogParser`

```cpp
#include "verilog_parser.h"
```

```cpp
VerilogParser parser;

// 从文件解析
bool success = parser.parse("/path/to/design.v", circuit);

// 从字符串解析（适合测试）
bool success = parser.parseString(verilog_source, circuit);

if (!success) {
    std::cerr << parser.getLastError() << "\n";
}
```

**局限：**
- 仅支持门级结构化 Verilog（不支持 RTL / `always` / `reg`）
- 模块尺寸使用内置简化映射（实际应从 LEF 文件读取）
- 每个文件仅支持解析单一 `module`

---

## 布局算法

### `PlacerBase`（基类）

```cpp
#include "placer_base.h"
```

```cpp
// 进度回调类型
using ProgressCallback = std::function<void(int iteration, double hpwl, double overflow)>;
```

```cpp
// 设置进度回调（可选）
placer.setProgressCallback([](int iter, double hpwl, double overflow) {
    std::cout << "iter=" << iter << " hpwl=" << hpwl << "\n";
});

// 执行布局
bool success = placer.run();

// 获取统计结果
const PlacementStats& stats = placer.getStats();
stats.print();

// 手动查询
double hpwl = placer.calculateHPWL();
double overflow = placer.calculateOverflow();
std::string error = placer.getLastError();
```

---

### `InitialPlacer`（Kraftwerk2A）

```cpp
#include "initial_placer.h"

InitialPlacer placer(circuit);
placer.setMaxIterations(100);     // BiCGSTAB 最大迭代次数（默认 100）
placer.setTargetError(1e-4);      // BiCGSTAB 收敛误差（默认 1e-4）
placer.run();
```

---

### `GlobalPlacer`（ePlace-MS 简化版）

```cpp
#include "global_placer.h"

GlobalPlacer placer(circuit);
placer.setTargetDensity(1.0);     // 目标密度（默认 1.0）
placer.setTargetOverflow(0.1);    // 收敛条件：overflow ≤ 此值（默认 0.1）
placer.setMaxIterations(200);     // 最大迭代次数（默认 200）
placer.setLearningRate(0.1);      // 梯度步长系数（默认 0.1）
placer.run();
```

---

### `PlacementStats`

```cpp
struct PlacementStats {
    double initial_hpwl;       // 布局前 HPWL
    double final_hpwl;         // 布局后 HPWL
    double hpwl_improvement;   // HPWL 改善率（%）
    double initial_overflow;   // 布局前密度溢出率
    double final_overflow;     // 布局后密度溢出率
    int iterations;            // 实际执行迭代次数
    double runtime_seconds;    // 执行时间（秒）

    void print() const;        // 打印到 stdout
};
```

---

## 可视化

### `PlacementVisualizer`

```cpp
#include "placement_visualizer.h"

PlacementVisualizer viz(circuit);

// 导出布局图（默认 1200×1200 像素）
viz.exportToBMP("placement.bmp");

// 自定义选项
VisualizerOptions options;
options.image_width   = 2400;
options.image_height  = 2400;
options.show_modules  = true;
options.show_nets     = true;       // 绘制连线（大规模设计较慢）
options.show_density  = true;       // 叠加密度热力图
options.density_bins_x = 64;
options.density_bins_y = 64;
options.movable_color = Color::Red();
options.fixed_color   = Color::Blue();
viz.exportToBMP("placement_detail.bmp", options);

// 仅导出密度热力图
viz.exportDensityMap("density.bmp", 64, 64);
```

### `VisualizerOptions` 默认值

| 字段 | 默认值 | 说明 |
|------|--------|------|
| `image_width` | 1200 | 图像宽度（像素） |
| `image_height` | 1200 | 图像高度（像素） |
| `margin` | 50 | 边距（像素） |
| `show_modules` | `true` | 绘制模块矩形 |
| `show_nets` | `false` | 绘制连线（性能影响大） |
| `show_density` | `false` | 叠加密度热力图 |
| `density_bins_x/y` | 64 | 密度网格分辨率 |
| `movable_color` | Red | 可移动单元颜色 |
| `fixed_color` | Blue | 固定端口颜色 |
| `macro_color` | Green | 宏单元颜色 |

---

## 枚举类型

### `ModuleType`

```cpp
enum class ModuleType {
    STANDARD_CELL,   // 可移动标准单元
    MACRO,           // 宏单元（大面积，可能固定）
    TERMINAL,        // I/O 端口（固定）
    FILLER,          // 填充单元（布局内部使用，导出时忽略）
    UNKNOWN
};
```

### `PinDirection`

```cpp
enum class PinDirection {
    INPUT,
    OUTPUT,
    INOUT,
    UNKNOWN
};
```

### `PlacerType`

```cpp
enum class PlacerType {
    INITIAL_KRAFTWERK2A,
    GLOBAL_EPLACE_MS,
    CUSTOM
};
```

---

## 完整示例

```cpp
#include "circuit_model.h"
#include "bookshelf_parser.h"
#include "initial_placer.h"
#include "global_placer.h"
#include "placement_visualizer.h"

using namespace eda;

int main() {
    // 1. 解析设计
    CircuitModel circuit;
    BookShelfParser parser;
    if (!parser.parse("design.aux", circuit)) {
        std::cerr << parser.getLastError() << "\n";
        return 1;
    }

    std::cout << "Modules: " << circuit.getNumModules() << "\n";
    std::cout << "Nets:    " << circuit.getNumNets() << "\n";

    // 2. 初始布局
    InitialPlacer init_placer(circuit);
    init_placer.run();
    init_placer.getStats().print();

    // 3. 全局布局
    GlobalPlacer global_placer(circuit);
    global_placer.setTargetOverflow(0.1);
    global_placer.setProgressCallback([](int iter, double hpwl, double overflow) {
        if (iter % 50 == 0) {
            std::cout << "iter=" << iter
                      << " hpwl=" << hpwl
                      << " overflow=" << overflow * 100.0 << "%\n";
        }
    });
    global_placer.run();
    global_placer.getStats().print();

    // 4. 导出结果
    circuit.exportToPl("result.pl");

    PlacementVisualizer viz(circuit);
    viz.exportToBMP("placement.bmp");
    viz.exportDensityMap("density.bmp");

    return 0;
}
```
