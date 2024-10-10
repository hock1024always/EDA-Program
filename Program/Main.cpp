#include <wx/wx.h>       // 包含 wxWidgets 库的头文件
#include <wx/sizer.h>    // 包含 wxSizer 类的头文件
#include <wx/dcbuffer.h> // 包含 wxBufferedPaintDC 类的头文件
#include <vector>        // 包含标准库的 vector 容器

// 定义 ShapeType 枚举类型，表示图形类型
enum class ShapeType { Rectangle, Circle };

// 定义 Shape 结构体，表示图形的位置和类型
struct Shape {
    ShapeType type; // 图形的类型（矩形或圆形）
    int x, y;       // 图形的位置坐标
};

// 定义 Line 结构体，表示线条的起点和终点
struct Line {
    int startX, startY, endX, endY; // 线条的起点和终点坐标
};

// 定义 DrawingPanel 类，继承自 wxPanel，用于处理绘图事件
class DrawingPanel : public wxPanel {
public:
    // 构造函数，初始化面板并绑定事件处理函数
    DrawingPanel(wxFrame* parent) : wxPanel(parent, wxID_ANY) {
        Bind(wxEVT_PAINT, &DrawingPanel::OnPaint, this);         // 绑定绘图事件
        Bind(wxEVT_LEFT_DOWN, &DrawingPanel::OnLeftDown, this);  // 绑定鼠标左键按下事件
        Bind(wxEVT_LEFT_UP, &DrawingPanel::OnLeftUp, this);      // 绑定鼠标左键释放事件
        Bind(wxEVT_MOTION, &DrawingPanel::OnMotion, this);       // 绑定鼠标移动事件
    }

    // 处理绘图事件，绘制图形和线条
    void OnPaint(wxPaintEvent& event) {
        wxBufferedPaintDC dc(this); // 使用双缓冲绘图以避免闪烁
        dc.Clear();                 // 清空绘图区域

        // 绘制所有图形
        for (const auto& shape : shapes) {
            if (shape.type == ShapeType::Rectangle) {
                dc.DrawRectangle(shape.x, shape.y, 50, 50); // 绘制矩形
            }
            else if (shape.type == ShapeType::Circle) {
                dc.DrawCircle(shape.x, shape.y, 25);        // 绘制圆形
            }
        }

        // 绘制所有线条
        for (const auto& line : lines) {
            dc.DrawLine(line.startX, line.startY, line.endX, line.endY); // 绘制线条
        }
    }

    // 处理鼠标左键按下事件
    void OnLeftDown(wxMouseEvent& event) {
        int mouseX = event.GetX(); // 获取鼠标点击的 X 坐标
        int mouseY = event.GetY(); // 获取鼠标点击的 Y 坐标

        // 检查是否点击了某个图形
        for (size_t i = 0; i < shapes.size(); ++i) {
            if (IsPointInShape(mouseX, mouseY, shapes[i])) {
                StartDrag(i, mouseX, mouseY); // 开始拖动图形
                return;
            }
        }

        // 如果没有点击图形，则开始绘制线条
        currentLine.startX = mouseX; // 设置线条的起点为鼠标点击位置
        currentLine.startY = mouseY;
    }

    // 处理鼠标左键释放事件
    void OnLeftUp(wxMouseEvent& event) {
        if (dragging) {
            dragging = false; // 结束图形拖动
        }
        else {
            // 结束绘制线条
            currentLine.endX = event.GetX(); // 设置线条的终点为鼠标释放位置
            currentLine.endY = event.GetY();
            lines.push_back(currentLine);    // 将线条添加到线条列表中
        }
        Refresh(); // 刷新面板以重新绘制
    }

    // 处理鼠标移动事件
    void OnMotion(wxMouseEvent& event) {
        if (dragging) {
            // 拖动图形
            shapes[dragIndex].x = event.GetX() - dragOffsetX; // 更新图形位置
            shapes[dragIndex].y = event.GetY() - dragOffsetY;
            Refresh(); // 刷新面板以重新绘制
        }
        else if (event.Dragging()) {
            // 绘制线条
            currentLine.endX = event.GetX(); // 更新线条的终点
            currentLine.endY = event.GetY();
            Refresh(); // 刷新面板以重新绘制
        }
    }

    // 添加图形到画板
    void AddShape(ShapeType type, int x, int y) {
        shapes.push_back({ type, x, y }); // 将图形添加到图形列表中
        Refresh(); // 刷新面板以重新绘制
    }

    // 开始拖动图形
    void StartDrag(int index, int x, int y) {
        dragIndex = index;                      // 设置当前拖动的图形索引
        dragOffsetX = x - shapes[index].x;      // 计算拖动偏移量
        dragOffsetY = y - shapes[index].y;
        dragging = true;                        // 设置为正在拖动状态
    }

private:
    std::vector<Shape> shapes; // 存储所有图形的列表
    std::vector<Line> lines;   // 存储所有线条的列表
    Line currentLine;          // 当前正在绘制的线条
    bool dragging = false;     // 是否正在拖动图形的标志
    int dragIndex = -1;        // 当前拖动的图形索引
    int dragOffsetX = 0, dragOffsetY = 0; // 拖动图形的偏移量

    // 检查鼠标点击位置是否在图形内
    bool IsPointInShape(int x, int y, const Shape& shape) {
        if (shape.type == ShapeType::Rectangle) {
            // 检查点是否在矩形内
            return x >= shape.x && x <= shape.x + 50 && y >= shape.y && y <= shape.y + 50;
        }
        else if (shape.type == ShapeType::Circle) {
            // 检查点是否在圆形内
            return (x - (shape.x + 25)) * (x - (shape.x + 25)) + (y - (shape.y + 25)) * (y - (shape.y + 25)) <= 25 * 25;
        }
        return false;
    }
};

// 定义 Sidebar 类，继承自 wxPanel，用于显示工具栏
class Sidebar : public wxPanel {
public:
    // 构造函数，初始化工具栏并绑定按钮事件
    Sidebar(wxFrame* parent, DrawingPanel* drawingPanel) : wxPanel(parent, wxID_ANY), drawingPanel(drawingPanel) {
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL); // 创建垂直布局的 sizer

        wxButton* rectButton = new wxButton(this, wxID_ANY, "Rectangle"); // 创建矩形按钮
        wxButton* circleButton = new wxButton(this, wxID_ANY, "Circle");   // 创建圆形按钮

        sizer->Add(rectButton, 0, wxALL, 5);  // 将矩形按钮添加到 sizer
        sizer->Add(circleButton, 0, wxALL, 5); // 将圆形按钮添加到 sizer

        SetSizer(sizer); // 设置面板的 sizer

        rectButton->Bind(wxEVT_BUTTON, &Sidebar::OnRectButton, this);  // 绑定矩形按钮事件
        circleButton->Bind(wxEVT_BUTTON, &Sidebar::OnCircleButton, this); // 绑定圆形按钮事件
    }

    // 处理矩形按钮点击事件
    void OnRectButton(wxCommandEvent& event) {
        drawingPanel->AddShape(ShapeType::Rectangle, 50, 50); // 添加矩形
    }

    // 处理圆形按钮点击事件
    void OnCircleButton(wxCommandEvent& event) {
        drawingPanel->AddShape(ShapeType::Circle, 50, 50); // 添加圆形
    }

private:
    DrawingPanel* drawingPanel; // 指向绘图面板的指针
};

// 定义 MainFrame 类，继承自 wxFrame，用于创建主窗口
class MainFrame : public wxFrame {
public:
    // 构造函数，初始化主窗口并设置布局
    MainFrame() : wxFrame(nullptr, wxID_ANY, "wxWidgets Drawing App") {
        wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL); // 创建水平布局的 sizer

        drawingPanel = new DrawingPanel(this); // 创建绘图面板
        sidebar = new Sidebar(this, drawingPanel); // 创建工具栏

        sizer->Add(sidebar, 0, wxEXPAND | wxALL, 5);  // 将工具栏添加到 sizer
        sizer->Add(drawingPanel, 1, wxEXPAND | wxALL, 5); // 将绘图面板添加到 sizer

        SetSizerAndFit(sizer); // 设置 sizer 并调整窗口大小
        SetSize(800, 600);     // 设置窗口大小
    }

private:
    DrawingPanel* drawingPanel; // 指向绘图面板的指针
    Sidebar* sidebar;           // 指向工具栏的指针
};

// 定义 MyApp 类，继承自 wxApp，用于应用程序的入口
class MyApp : public wxApp {
public:
    // 应用程序初始化函数
    virtual bool OnInit() {
        MainFrame* frame = new MainFrame(); // 创建主窗口
        frame->Show(true);                  // 显示主窗口
        return true;                        // 返回 true 表示初始化成功
    }
};

// 实现应用程序入口
wxIMPLEMENT_APP(MyApp);