#include "PointGrid.h"

// 构造函数，初始化点状图的面板和间距
PointGrid::PointGrid(wxPanel* panel, int spacing) : panel(panel), spacing(spacing) {}

// 绘制点状图
void PointGrid::Draw(wxBufferedPaintDC& dc) {
    wxBrush brush(wxColour(200, 200, 200)); // 点的颜色
    dc.SetBrush(brush);

    // 绘制点状背景
    for (int x = 0; x < panel->GetSize().GetWidth(); x += spacing) {
        for (int y = 0; y < panel->GetSize().GetHeight(); y += spacing) {
            dc.DrawCircle(x, y, 2); // 绘制小圆点
        }
    }
}

