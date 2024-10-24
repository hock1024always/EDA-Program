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
        // 正方形的左上角为 (shape.x, shape.y)，宽高为 40
        return x >= shape.x - 20 && x <= shape.x + 20 && y >= shape.y -20 && y <= shape.y + 20;
    case ShapeType::OrGate:
        // 圆的中心为 (shape.x , shape.y )，半径为 20
        return (x - shape.x ) * (x - shape.x ) + (y - shape.y ) * (y - shape.y) <= 20 * 20;
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
        return (x - shape.x) * (x - shape.x) + (y - shape.y) * (y - shape.y) <= 10 * 10;
    case ShapeType::OffPin: {
        return x >= shape.x - 10 && x <= shape.x + 10 && y >= shape.y - 10 && y <= shape.y + 10;
    }
    default:
        return false;
    }
}

