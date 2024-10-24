# 源代码及设计思路

## 程序入口

### ```Main.cpp```

提供一个简单的程序入口

```c++
#include <wx/wx.h>
#include "MainFrame.h"

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
```

## 项目外框实现

这里面提供了两个部分连接的基本结构，设置了程序的总外框大小，以及调用了sidebar里面的构造函数，设置了侧边框的宽度

### ```MainFrame.h```

```cpp
#pragma once
#ifndef MAINFRAME_H
#define MAINFRAME_H

#include <wx/wx.h>
#include "DrawingPanel.h"
#include "Sidebar.h"

// 定义 MainFrame 类，继承自 wxFrame，用于创建主窗口
class MainFrame : public wxFrame {
public:
    MainFrame(); // 构造函数

private:
    DrawingPanel* drawingPanel; // 指向绘图面板的指针
    Sidebar* sidebar;           // 指向工具栏的指针
};

#endif // MAINFRAME_H
```

### ```MainFrame.cpp```

```cpp
#include "MainFrame.h"

// 构造函数，初始化主窗口并设置布局
MainFrame::MainFrame() : wxFrame(nullptr, wxID_ANY, "wxWidgets Drawing App") {
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL); // 创建水平布局的 sizer

    drawingPanel = new DrawingPanel(this); // 创建绘图面板
    sidebar = new Sidebar(this, drawingPanel, 200); // 假设原来的宽度是 100，现在设置为 200

    sizer->Add(sidebar, 0, wxEXPAND | wxALL, 5);  // 将工具栏添加到 sizer
    sizer->Add(drawingPanel, 1, wxEXPAND | wxALL, 5); // 将绘图面板添加到 sizer

    SetSizerAndFit(sizer); // 设置 sizer 并调整窗口大小
    SetSize(800, 600);     // 设置窗口大小
}
```

## 侧边框以及属性表

1. 生成了按钮，指代五个元件
2. 每个按钮绑定了两个事件，分别是向画板释放元件，以及更新侧边框内容
3. 侧边框是写死的，就是打印，之后需要更新

### ```Sidebar.h```

注释是之前生成普通画板时候的，懒得改了

```cpp
#pragma once
#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <wx/wx.h>
#include "DrawingPanel.h"

// 定义 Sidebar 类，继承自 wxPanel，用于显示工具栏
class Sidebar : public wxPanel {
public:
    Sidebar(wxFrame* parent, DrawingPanel* drawingPanel, int width); // 构造函数

private:
    void OnRectButton(wxCommandEvent& event);      // 处理矩形按钮点击事件
    void OnCircleButton(wxCommandEvent& event);    // 处理圆形按钮点击事件
    void OnTriangleButton(wxCommandEvent& event);  // 处理三角形按钮点击事件
    void OnEllipseButton(wxCommandEvent& event);    // 处理椭圆按钮点击事件
    void OnDiamondButton(wxCommandEvent& event);    // 处理菱形按钮点击事件

    wxStaticText* infoText; // 用于显示释放的图形信息
    DrawingPanel* drawingPanel; // 指向绘图面板的指针
};

#endif // SIDEBAR_H
```

### ```Sidebar.cpp```

```cpp
#include "Sidebar.h"

// 构造函数，初始化工具栏并绑定按钮事件
Sidebar::Sidebar(wxFrame* parent, DrawingPanel* drawingPanel, int width)
    : wxPanel(parent, wxID_ANY), drawingPanel(drawingPanel) {
    SetMinSize(wxSize(width, -1)); // 设置最小宽度

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    wxButton* rectButton = new wxButton(this, wxID_ANY, "与门"); // Rectangle
    wxButton* circleButton = new wxButton(this, wxID_ANY, "或门"); // Circle
    wxButton* triangleButton = new wxButton(this, wxID_ANY, "非门"); // Triangle
    wxButton* ellipseButton = new wxButton(this, wxID_ANY, "1信号输入"); // Ellipse
    wxButton* diamondButton = new wxButton(this, wxID_ANY, "0信号输入"); // Diamond

    // 创建静态文本，用于显示图形释放信息
    infoText = new wxStaticText(this, wxID_ANY, "元件属性表");
    infoText->SetMinSize(wxSize(-1, 50)); // 设置文本区域的最小高度

    sizer->Add(rectButton, 0, wxALL, 5);
    sizer->Add(circleButton, 0, wxALL, 5);
    sizer->Add(triangleButton, 0, wxALL, 5);
    sizer->Add(ellipseButton, 0, wxALL, 5);
    sizer->Add(diamondButton, 0, wxALL, 5);
    sizer->Add(infoText, 0, wxALL | wxEXPAND, 5); // 添加信息显示区域

    SetSizer(sizer);

    rectButton->Bind(wxEVT_BUTTON, &Sidebar::OnRectButton, this);
    circleButton->Bind(wxEVT_BUTTON, &Sidebar::OnCircleButton, this);
    triangleButton->Bind(wxEVT_BUTTON, &Sidebar::OnTriangleButton, this);
    ellipseButton->Bind(wxEVT_BUTTON, &Sidebar::OnEllipseButton, this);
    diamondButton->Bind(wxEVT_BUTTON, &Sidebar::OnDiamondButton, this);
}

void Sidebar::OnRectButton(wxCommandEvent& event) {
    drawingPanel->AddShape(ShapeType::AndGate, 50, 50);
    wxString info = "名称: 与门\n接入口数: 2\n输出口数: 1\n功能: 双真出真";
    infoText->SetLabel(info);
}

void Sidebar::OnCircleButton(wxCommandEvent& event) {
    drawingPanel->AddShape(ShapeType::OrGate, 150, 100);
    infoText->SetLabel("名称: 或门\n接入口数: 2\n输出口数: 1\n功能: 有真出真");
}

void Sidebar::OnTriangleButton(wxCommandEvent& event) {
    drawingPanel->AddShape(ShapeType::NotGate, 50, 100);
    infoText->SetLabel("名称: 非门\n接入口数: 2\n输出口数: 1\n功能: 双假出真");
}

void Sidebar::OnEllipseButton(wxCommandEvent& event) {
    drawingPanel->AddShape(ShapeType::OnPin, 150, 150);
    infoText->SetLabel("名称: 1输入设备\n接入口数: 0\n输出口数: 1\n功能: 输出1");
}

void Sidebar::OnDiamondButton(wxCommandEvent& event) {
    drawingPanel->AddShape(ShapeType::OffPin, 100, 150);
    infoText->SetLabel("名称: 0输入设备\n接入口数: 0\n输出口数: 1\n功能: 输出0");
}
```

## 画板基础信息

### ```Shape.h```

枚举定义元件，以及线条的起止点

```cpp
#pragma once
#ifndef SHAPE_H
#define SHAPE_H

// 定义 ShapeType 枚举类型，表示图形类型
enum class ShapeType {
    AndGate, 
    OrGate, 
    NotGate, 
    OnPin, 
    OffPin 
};

// 定义 Shape 结构体，表示图形的位置和类型
struct Shape {
    ShapeType type; // 图形的类型
    int x, y;       // 图形的位置坐标
};

// 定义 Line 结构体，表示线条的起点和终点
struct Line {
    int startX, startY, endX, endY; // 线条的起点和终点坐标
};

#endif // SHAPE_H
```

### ```PointGrid.h```

文件名比较奇怪，主要是之前点状图确实不合适使用。

实现了网格间距的定义，以及绘制背景图的类

```cpp
#ifndef POINTGRID_H
#define POINTGRID_H

#include <wx/wx.h>
#include <wx/dcbuffer.h> // 添加这个头文件以使用 wxBufferedPaintDC

// 定义 PointGrid 类，用于绘制网格背景
class PointGrid {
public:
    PointGrid(wxPanel* panel, int spacing);
    void Draw(wxBufferedPaintDC& dc);

private:
    wxPanel* panel; // 指向绘图面板的指针
    int spacing;    // 网格的间距
};

#endif // POINTGRID_H
```

## 画板（star）

1. 实现了图形的绘制，包括轮廓和锚点（期中非门的定义法是我们之后设计原件的主要方法）
2. 处理了鼠标的左点击，移动，左释放事件，使用的是三段横平竖直的线段来连接
3. 线段的起点尽量捕捉到锚点上，但是条件有点乱，导致精确性不高，之后需要更改
4. 实现了鼠标点击在图形中可以拖动的事件，主要是后两个元件比较灵敏

### ```DrawingPanel.h```

```
#pragma once
#ifndef DRAWINGPANEL_H
#define DRAWINGPANEL_H

#include <wx/wx.h>
#include <vector>
#include "Shape.h"
#include "PointGrid.h"
#include <cmath>  // 用于计算平方根和平方

// 定义锚点结构
struct AnchorPoint {
    int x;
    int y;

    AnchorPoint(int x, int y) : x(x), y(y) {}
};

// 定义 DrawingPanel 类，继承自 wxPanel，用于处理绘图事件
class DrawingPanel : public wxPanel {
public:
    DrawingPanel(wxFrame* parent); // 构造函数

    void OnPaint(wxPaintEvent& event); // 处理绘图事件
    void OnLeftDown(wxMouseEvent& event); // 处理鼠标左键按下事件
    void OnLeftUp(wxMouseEvent& event); // 处理鼠标左键释放事件
    void OnMotion(wxMouseEvent& event); // 处理鼠标移动事件
    void AddShape(ShapeType type, int x, int y); // 添加图形到画板

private:
    PointGrid pointGrid; // 点状图的对象
    std::vector<Shape> shapes; // 存储所有图形的列表
    std::vector<Line> lines; // 存储所有线条的列表
    std::vector<AnchorPoint> anchorPoints; // 存储锚点的列表
    Line currentLine; // 当前正在绘制的线条
    bool dragging = false; // 是否正在拖动图形的标志
    int dragIndex = -1; // 当前拖动的图形索引
    int dragOffsetX = 0, dragOffsetY = 0; // 拖动图形的偏移量

    // 新增常量：锚点的判断域半径
    const int anchorRadius = 5;

    void StartDrag(int index, int x, int y); // 开始拖动图形
    bool IsPointInShape(int x, int y, const Shape& shape); // 检查鼠标点击位置是否在图形内
    bool IsPointOnAnchor(int x, int y); // 检查点是否在锚点上

    void SnapToPoint(int& x, int& y); // 将坐标对齐到最近的点状图上
    std::vector<Line> CalculateSegments(int startX, int startY, int endX, int endY); // 计算最多三条平行于网格边的线段

};

#endif // DRAWINGPANEL_H
```



### ```DrawingPanel.cpp```

```cpp
#include "DrawingPanel.h"
#include <cmath>  // 用于计算平方根和平方
#include <algorithm> // 用于 std::min 和 std::max

// 构造函数，初始化绘图面板并创建点状图
DrawingPanel::DrawingPanel(wxFrame* parent)
    : wxPanel(parent, wxID_ANY), pointGrid(this, 10) { // 设置点的间距为10像素
    Bind(wxEVT_PAINT, &DrawingPanel::OnPaint, this);
    Bind(wxEVT_LEFT_DOWN, &DrawingPanel::OnLeftDown, this);
    Bind(wxEVT_LEFT_UP, &DrawingPanel::OnLeftUp, this);
    Bind(wxEVT_MOTION, &DrawingPanel::OnMotion, this);
}

void DrawingPanel::OnPaint(wxPaintEvent& event) {
    wxBufferedPaintDC dc(this);
    dc.Clear();

    pointGrid.Draw(dc); // 绘制点状图

    for (const auto& shape : shapes) {
    // 设置黄色的画笔和画刷
    dc.SetPen(wxPen(wxColour(255, 255, 0))); // 黄色画笔
    dc.SetBrush(wxBrush(wxColour(255, 255, 0))); // 黄色画刷

    switch (shape.type) {
    case ShapeType::AndGate:
        // 正方形的中心点为 shape.x, shape.y，边长为 40
        dc.DrawRectangle(shape.x - 20, shape.y - 20, 40, 40);
        // 添加黑色实心小圆
        dc.SetPen(*wxBLACK_PEN); // 恢复黑色画笔
        dc.SetBrush(*wxBLACK_BRUSH); // 恢复黑色画刷
        dc.DrawCircle(shape.x - 20, shape.y + 10, 3);
        dc.DrawCircle(shape.x - 20, shape.y - 10, 3);
        dc.DrawCircle(shape.x + 20, shape.y, 3);
        break;
    case ShapeType::OrGate:
        // 圆的中心点为 shape.x, shape.y，半径为 20
        dc.DrawCircle(shape.x, shape.y, 20);
        // 添加黑色实心小圆
        dc.SetPen(*wxBLACK_PEN); // 恢复黑色画笔
        dc.SetBrush(*wxBLACK_BRUSH); // 恢复黑色画刷
        dc.DrawCircle(shape.x - 20, shape.y + 10, 3);
        dc.DrawCircle(shape.x - 20, shape.y - 10, 3);
        dc.DrawCircle(shape.x + 20, shape.y, 3);
        break;
    case ShapeType::NotGate: {
        // 等腰直角三角形的顶点为 shape.x, shape.y，斜边竖直，斜边长 40
        wxPoint points[3] = {
            wxPoint(shape.x, shape.y),
            wxPoint(shape.x - 20, shape.y + 20),
            wxPoint(shape.x - 20, shape.y - 20)
        };
        dc.DrawPolygon(3, points);
        // 添加黑色实心小圆
        dc.SetPen(*wxBLACK_PEN); // 恢复黑色画笔
        dc.SetBrush(*wxBLACK_BRUSH); // 恢复黑色画刷
        dc.DrawCircle(shape.x - 20, shape.y + 10, 3);
        dc.DrawCircle(shape.x - 20, shape.y - 10, 3);
        dc.DrawCircle(shape.x, shape.y, 3);
        break;
    }
    case ShapeType::OnPin:
        // 圆的中心点为 shape.x, shape.y，半径为 10
        dc.DrawCircle(shape.x, shape.y, 10);
        // 添加黑色实心小圆
        dc.SetPen(*wxBLACK_PEN); // 恢复黑色画笔
        dc.SetBrush(*wxBLACK_BRUSH); // 恢复黑色画刷
        dc.DrawCircle(shape.x + 10, shape.y, 3);
        break;
    case ShapeType::OffPin: {
        // 正方形的中心点为 shape.x, shape.y，边长为 20
        dc.DrawRectangle(shape.x - 10, shape.y - 10, 20, 20);
        // 添加黑色实心小圆
        dc.SetPen(*wxBLACK_PEN); // 恢复黑色画笔
        dc.SetBrush(*wxBLACK_BRUSH); // 恢复黑色画刷
        dc.DrawCircle(shape.x + 10, shape.y, 3);
        break;
    }
    }

    // 恢复默认的画笔和刷子，如果需要的话
    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(*wxWHITE_BRUSH);
}

    for (const auto& line : lines) {
        dc.DrawLine(line.startX, line.startY, line.endX, line.endY);
    }
}

// 检查是否在锚点的判断域内（圆形区域）
bool DrawingPanel::IsPointOnAnchor(int x, int y) {
    for (const auto& anchor : anchorPoints) {
        // 计算点击点与锚点的距离
        double distance = std::sqrt(std::pow(x - anchor.x, 2) + std::pow(y - anchor.y, 2));

        // 如果距离小于等于锚点的判断域半径，则认为在锚点上
        if (distance <= anchorRadius) {
            return true;
        }
    }
    return false;
}



// 处理鼠标左键按下事件
void DrawingPanel::OnLeftDown(wxMouseEvent& event) {
    // 获取鼠标当前的坐标
    int mouseX = event.GetX();
    int mouseY = event.GetY();

    // 将鼠标坐标对齐到点状网格（每10个像素一个点）
    SnapToPoint(mouseX, mouseY);

    // 检查是否点击在某个锚点的判断域内
    if (IsPointOnAnchor(mouseX, mouseY)) {
        // 如果点击在锚点上，记录锚点坐标为起点
        currentLine.startX = mouseX;
        currentLine.startY = mouseY;
        return; // 退出函数，因为已经开始绘制线
    }

    // 遍历所有图形，检查鼠标是否在某个图形内部
    for (size_t i = 0; i < shapes.size(); ++i) {
        // 检查鼠标是否在第i个图形内
        if (IsPointInShape(mouseX, mouseY, shapes[i])) {
            // 如果鼠标在图形内，开始拖动该图形
            StartDrag(i, mouseX, mouseY);
            return; // 退出函数，因为已经开始拖动
        }
    }

    // 如果没有点击到任何图形或锚点，开始绘制新的线
    currentLine.startX = mouseX; // 记录线的起点
    currentLine.startY = mouseY;
}

//// 处理鼠标左键释放事件
//void DrawingPanel::OnLeftUp(wxMouseEvent& event) {
//    // 如果当前正在拖动图形，结束拖动
//    if (dragging) {
//        dragging = false; // 结束拖动状态
//    }
//    else {
//        // 如果没有拖动图形，则处理线的绘制
//        int endX = event.GetX(); // 记录线的终点
//        int endY = event.GetY();
//        // 将终点坐标对齐到点状网格
//        SnapToPoint(endX, endY);
//
//        // 检查是否释放位置在某个锚点上
//        if (IsPointOnAnchor(endX, endY)) {
//            // 如果释放位置在锚点上，则将终点设置为这个锚点的坐标
//            endX = currentLine.startX;
//            endY = currentLine.startY;
//        }
//
//        // 计算最多三条平行于网格边的线段
//        std::vector<Line> segments = CalculateSegments(currentLine.startX, currentLine.startY, endX, endY);
//        // 将计算出的线段添加到线段列表中
//        lines.insert(lines.end(), segments.begin(), segments.end());
//    }
//    // 刷新面板以更新显示
//    Refresh();
//}

// 处理鼠标左键释放事件
void DrawingPanel::OnLeftUp(wxMouseEvent& event) {
    // 如果当前正在拖动图形，结束拖动
    if (dragging) {
        dragging = false; // 结束拖动状态
    }
    else {
        // 如果没有拖动图形，则处理线的绘制
        int endX = event.GetX(); // 记录线的终点
        int endY = event.GetY();

        // 将终点坐标对齐到点状网格
        SnapToPoint(endX, endY);

        // 检查是否释放位置在某个锚点上
        if (IsPointOnAnchor(endX, endY)) {
            // 如果释放位置在锚点上，则将终点设置为这个锚点的坐标
            endX = currentLine.startX;
            endY = currentLine.startY;
        }

        // 计算最多三条平行于网格边的线段
        std::vector<Line> segments = CalculateSegments(currentLine.startX, currentLine.startY, endX, endY);
        // 将计算出的线段添加到线段列表中
        lines.insert(lines.end(), segments.begin(), segments.end());
    }
    // 刷新面板以更新显示
    Refresh();
}

// 处理鼠标移动事件
void DrawingPanel::OnMotion(wxMouseEvent& event) {
    // 如果当前处于拖动状态
    if (dragging) {
        // 更新被拖动图形的坐标
        shapes[dragIndex].x = event.GetX() - dragOffsetX;
        shapes[dragIndex].y = event.GetY() - dragOffsetY;
        // 将图形坐标对齐到点状网格
        SnapToPoint(shapes[dragIndex].x, shapes[dragIndex].y);
        // 刷新面板以更新显示
        Refresh();
    }
    else if (event.Dragging()) { // 如果没有拖动图形，但鼠标在拖动中（绘制线过程）
        // 更新当前正在绘制的线的终点坐标
        currentLine.endX = event.GetX();
        currentLine.endY = event.GetY();
        // 将终点坐标对齐到点状网格
        SnapToPoint(currentLine.endX, currentLine.endY);
        // 刷新面板以更新显示
        Refresh();
    }
}

// 将坐标对齐到点状网格
void DrawingPanel::SnapToPoint(int& x, int& y) {
    // 将 x 和 y 坐标对齐到最近的网格点（每10个像素一个点）
    x = (x / 10) * 10;
    y = (y / 10) * 10;
}



// 计算最多三条平行于网格边的线段
std::vector<Line> DrawingPanel::CalculateSegments(int startX, int startY, int endX, int endY) {
    std::vector<Line> segments;

    // 计算水平和垂直距离
    int dx = endX - startX;
    int dy = endY - startY;

    // 如果已经是一条水平或垂直线，直接返回
    if (dx == 0 || dy == 0) {
        segments.push_back({ startX, startY, endX, endY });
        return segments;
    }

    // 计算中点
    int midX = startX + dx / 2;

    // 对齐到网格
    SnapToPoint(midX, startY);
    SnapToPoint(midX, endY);

    // 添加三条线段
    segments.push_back({ startX, startY, midX, startY }); // 线段1：(a, b) -> (midx, b)
    segments.push_back({ midX, startY, midX, endY });   // 线段2：(midx, b) -> (midx, d)
    segments.push_back({ midX, endY, endX, endY });     // 线段3：(midx, d) -> (c, d)

    return segments;
}

// 添加新的图形
void DrawingPanel::AddShape(ShapeType type, int x, int y) {
    // 将新图形的坐标对齐到点状网格
    SnapToPoint(x, y);
    // 将新图形添加到图形列表中
    shapes.push_back({ type, x, y });
    // 刷新面板以更新显示
    Refresh();
}

// 开始拖动某个图形
void DrawingPanel::StartDrag(int index, int x, int y) {
    // 记录当前拖动的图形索引
    dragIndex = index;
    // 计算鼠标相对于图形左上角的偏移量
    dragOffsetX = x - shapes[index].x;
    dragOffsetY = y - shapes[index].y;
    // 设置拖动状态
    dragging = true;
}


bool DrawingPanel::IsPointInShape(int x, int y, const Shape& shape) {
    switch (shape.type) {
    case ShapeType::AndGate:
        // 正方形的左上角为 (shape.x, shape.y)，宽高为 50
        return x >= shape.x && x <= shape.x + 50 && y >= shape.y && y <= shape.y + 50;
    case ShapeType::OrGate:
        // 圆的中心为 (shape.x + 25, shape.y + 25)，半径为 25
        return (x - (shape.x + 25)) * (x - (shape.x + 25)) + (y - (shape.y + 25)) * (y - (shape.y + 25)) <= 25 * 25;
    case ShapeType::NotGate: {
        // 三角形的三个顶点
        wxPoint p(x, y);
        wxPoint a(shape.x, shape.y - 20); // 上顶点
        wxPoint b(shape.x - 20, shape.y + 20); // 左下顶点
        wxPoint c(shape.x + 20, shape.y + 20); // 右下顶点

        // 判断点是否在三角形内
        bool inside = false;
        int A = (b.y - a.y) * (c.x - b.x) + (b.x - a.x) * (c.y - b.y);
        int B1 = (b.y - p.y) * (c.x - b.x) + (b.x - p.x) * (c.y - b.y);
        int B2 = (c.y - b.y) * (a.x - c.x) + (c.x - b.x) * (a.y - c.y);
        int C1 = (p.y - a.y) * (b.x - a.x) + (a.x - p.x) * (b.y - a.y);
        int C2 = (c.y - a.y) * (p.x - c.x) + (a.x - c.x) * (p.y - c.y);

        if (A >= 0) {
            if (B1 >= 0 && B2 >= 0 && C1 >= 0 && C2 >= 0) inside = true;
        }
        else {
            if (B1 <= 0 && B2 <= 0 && C1 <= 0 && C2 <= 0) inside = true;
        }
        return inside;
    }
    case ShapeType::OnPin:
        // 椭圆，半长轴为 5，半短轴为 3.5
        return ((x - shape.x) * (x - shape.x)) / (5 * 5) + ((y - shape.y) * (y - shape.y)) / (3.5 * 3.5) <= 1;
    case ShapeType::OffPin: {
        // 菱形，中心为 (shape.x, shape.y)，对角线的一半为 10
        int dx = abs(x - shape.x);
        int dy = abs(y - shape.y);
        return dx * 1.414 + dy <= 20; // 近似菱形，1.414是sqrt(2)的近似值
    }
    default:
        return false;
    }
}
```

