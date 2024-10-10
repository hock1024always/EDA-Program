#pragma once
#ifndef DRAWINGPANEL_H
#define DRAWINGPANEL_H

#include <wx/wx.h>
#include <vector>
#include "Shape.h"
#include "PointGrid.h"

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
    Line currentLine; // 当前正在绘制的线条
    bool dragging = false; // 是否正在拖动图形的标志
    int dragIndex = -1; // 当前拖动的图形索引
    int dragOffsetX = 0, dragOffsetY = 0; // 拖动图形的偏移量

    void StartDrag(int index, int x, int y); // 开始拖动图形
    bool IsPointInShape(int x, int y, const Shape& shape); // 检查鼠标点击位置是否在图形内
    void SnapToPoint(int& x, int& y); // 将坐标对齐到最近的点状图上
};

#endif // DRAWINGPANEL_H
