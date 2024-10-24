#include "DrawingPanel.h"

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

    pointGrid.Draw(dc); // 绘制网格图

    for (const auto& shape : shapes) {
        switch (shape.type) {
        case ShapeType::OnPin:
            dc.DrawRectangle(shape.x - 10, shape.y - 10, 20, 20); // 正方形
            break;
        case ShapeType::OffPin:
            dc.DrawCircle(shape.x, shape.y, 5); // 圆形
            break;
        case ShapeType::NotGate: {
            wxPoint points[3] = {
                wxPoint(shape.x, shape.y - 10),
                wxPoint(shape.x - 10, shape.y + 10),
                wxPoint(shape.x + 10, shape.y + 10)
            };
            dc.DrawPolygon(3, points); // 等腰三角形
            break;
        }
        case ShapeType::OrGate: {
            dc.DrawEllipse(shape.x - 20, shape.y - 20, 40, 40); // 半圆形绘制
            break;
        }
        case ShapeType::AndGate: {
            dc.DrawRectangle(shape.x - 20, shape.y - 20, 40, 40); // 正方形
            break;
        }
        }
    }

    // 绘制线条
    wxPen linePen(wxColour(0, 0, 0), 2); // 设置线条颜色为黑色
    dc.SetPen(linePen);
    for (const auto& line : lines) {
        dc.DrawLine(line.startX, line.startY, line.endX, line.endY);
    }

    // 如果正在绘制线条，绘制当前线条
    if (currentLine.startX != currentLine.endX || currentLine.startY != currentLine.endY) {
        dc.DrawLine(currentLine.startX, currentLine.startY, currentLine.endX, currentLine.endY);
    }
}

void DrawingPanel::OnLeftDown(wxMouseEvent& event) {
    int mouseX = event.GetX();
    int mouseY = event.GetY();
    SnapToPoint(mouseX, mouseY); // 将坐标对齐到点状图

    // 检查是否点击了 pin 元件
    for (size_t i = 0; i < shapes.size(); ++i) {
        if (IsPointInShape(mouseX, mouseY, shapes[i])) {
            // 捕捉到了一个 pin 元件，开始绘制线条
            currentLine.startX = shapes[i].x + 10; // Pin 的锚点位置
            currentLine.startY = shapes[i].y; // Pin 的锚点位置
            currentLine.endX = mouseX; // 初始线条的结束位置
            currentLine.endY = mouseY;
            return;
        }
    }
}

void DrawingPanel::OnLeftUp(wxMouseEvent& event) {
    if (currentLine.startX == currentLine.endX && currentLine.startY == currentLine.endY) {
        // 如果线条没有移动，则不处理
        return;
    }

    currentLine.endX = event.GetX();
    currentLine.endY = event.GetY();
    SnapToPoint(currentLine.endX, currentLine.endY); // 对齐到点状图
    lines.push_back(currentLine); // 记录线条
    currentLine.startX = currentLine.endX; // 重置线条
    currentLine.startY = currentLine.endY;
    Refresh();
}

void DrawingPanel::OnMotion(wxMouseEvent& event) {
    if (event.Dragging()) {
        currentLine.endX = event.GetX();
        currentLine.endY = event.GetY();
        SnapToPoint(currentLine.endX, currentLine.endY); // 对齐到点状图
        Refresh();
    }
}

void DrawingPanel::SnapToPoint(int& x, int& y) {
    x = (x / 20) * 20; // 将 x 坐标对齐到最近的点
    y = (y / 20) * 20; // 将 y 坐标对齐到最近的点
}

void DrawingPanel::AddShape(ShapeType type, int x, int y) {
    SnapToPoint(x, y); // 新图形坐标对齐到点
    shapes.push_back({ type, x, y });
    Refresh();
}

void DrawingPanel::StartDrag(int index, int x, int y) {
    dragIndex = index;
    dragOffsetX = x - shapes[index].x;
    dragOffsetY = y - shapes[index].y;
    dragging = true;
}

//判定鼠标位置是否在图形内
bool DrawingPanel::IsPointInShape(int x, int y, const Shape& shape) {
    switch (shape.type) {
    case ShapeType::OnPin:
        return x >= shape.x - 10 && x <= shape.x + 10 && y >= shape.y - 10 && y <= shape.y + 10; // 检查在正方形内
    case ShapeType::OffPin:
        return (x - shape.x) * (x - shape.x) + (y - shape.y) * (y - shape.y) <= 5 * 5; // 检查在圆形内
    case ShapeType::NotGate: {
        // 简化判断三角形内部的逻辑
        return false; // 需要具体实现
    }
    case ShapeType::OrGate: {
        // 检查半圆形内的逻辑，需实现
        return false; // 需要具体实现
    }
    case ShapeType::AndGate:
        return x >= shape.x - 20 && x <= shape.x + 20 && y >= shape.y - 20 && y <= shape.y + 20; // 检查在正方形内
    default:
        return false;
    }
}



