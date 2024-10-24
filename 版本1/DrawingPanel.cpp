#include "DrawingPanel.h"

// ���캯������ʼ����ͼ��岢������״ͼ
DrawingPanel::DrawingPanel(wxFrame* parent)
    : wxPanel(parent, wxID_ANY), pointGrid(this, 10) { // ���õ�ļ��Ϊ10����
    Bind(wxEVT_PAINT, &DrawingPanel::OnPaint, this);
    Bind(wxEVT_LEFT_DOWN, &DrawingPanel::OnLeftDown, this);
    Bind(wxEVT_LEFT_UP, &DrawingPanel::OnLeftUp, this);
    Bind(wxEVT_MOTION, &DrawingPanel::OnMotion, this);
}

void DrawingPanel::OnPaint(wxPaintEvent& event) {
    wxBufferedPaintDC dc(this);
    dc.Clear();

    pointGrid.Draw(dc); // ���Ƶ�״ͼ

    for (const auto& shape : shapes) {
        switch (shape.type) {
        case ShapeType::Rectangle:
            dc.DrawRectangle(shape.x, shape.y, 50, 50);
            break;
        case ShapeType::Circle:
            dc.DrawCircle(shape.x, shape.y, 25);
            break;
        case ShapeType::Triangle: {
            wxPoint points[3] = {
                wxPoint(shape.x, shape.y - 25),
                wxPoint(shape.x - 25, shape.y + 25),
                wxPoint(shape.x + 25, shape.y + 25)
            };
            dc.DrawPolygon(3, points);
            break;
        }
        case ShapeType::Ellipse:
            dc.DrawEllipse(shape.x - 25, shape.y - 15, 50, 30);
            break;
        case ShapeType::Diamond: {
            wxPoint points[4] = {
                wxPoint(shape.x, shape.y - 25),
                wxPoint(shape.x - 25, shape.y),
                wxPoint(shape.x, shape.y + 25),
                wxPoint(shape.x + 25, shape.y)
            };
            dc.DrawPolygon(4, points);
            break;
        }
        }
    }

    for (const auto& line : lines) {
        dc.DrawLine(line.startX, line.startY, line.endX, line.endY);
    }
}

void DrawingPanel::OnLeftDown(wxMouseEvent& event) {
    int mouseX = event.GetX();
    int mouseY = event.GetY();
    SnapToPoint(mouseX, mouseY); // ��������뵽��״ͼ

    for (size_t i = 0; i < shapes.size(); ++i) {
        if (IsPointInShape(mouseX, mouseY, shapes[i])) {
            StartDrag(i, mouseX, mouseY);
            return;
        }
    }

    currentLine.startX = mouseX;
    currentLine.startY = mouseY;
}

void DrawingPanel::OnLeftUp(wxMouseEvent& event) {
    if (dragging) {
        dragging = false;
    }
    else {
        currentLine.endX = event.GetX();
        currentLine.endY = event.GetY();
        SnapToPoint(currentLine.endX, currentLine.endY); // ���뵽��״ͼ
        lines.push_back(currentLine);
    }
    Refresh();
}

void DrawingPanel::OnMotion(wxMouseEvent& event) {
    if (dragging) {
        shapes[dragIndex].x = event.GetX() - dragOffsetX;
        shapes[dragIndex].y = event.GetY() - dragOffsetY;
        SnapToPoint(shapes[dragIndex].x, shapes[dragIndex].y); // ���뵽��״ͼ
        Refresh();
    }
    else if (event.Dragging()) {
        currentLine.endX = event.GetX();
        currentLine.endY = event.GetY();
        SnapToPoint(currentLine.endX, currentLine.endY); // ���뵽��״ͼ
        Refresh();
    }
}

void DrawingPanel::SnapToPoint(int& x, int& y) {
    x = (x / 20) * 20; // �� x ������뵽����ĵ�
    y = (y / 20) * 20; // �� y ������뵽����ĵ�
}

void DrawingPanel::AddShape(ShapeType type, int x, int y) {
    SnapToPoint(x, y); // ��ͼ��������뵽��
    shapes.push_back({ type, x, y });
    Refresh();
}

void DrawingPanel::StartDrag(int index, int x, int y) {
    dragIndex = index;
    dragOffsetX = x - shapes[index].x;
    dragOffsetY = y - shapes[index].y;
    dragging = true;
}


//�ж�
bool DrawingPanel::IsPointInShape(int x, int y, const Shape& shape) {
    switch (shape.type) {
    case ShapeType::Rectangle:
        return x >= shape.x && x <= shape.x + 50 && y >= shape.y && y <= shape.y + 50;
    case ShapeType::Circle:
        return (x - (shape.x + 25)) * (x - (shape.x + 25)) + (y - (shape.y + 25)) * (y - (shape.y + 25)) <= 25 * 25;
    case ShapeType::Triangle: {
        // ȷ�����Ƿ����������ڵļ��㷽��������ʹ�����ķ���
        return false; // �򻯣���ʵ��ʵ���߼�
    }
    case ShapeType::Ellipse:
        return ((x - shape.x) * (x - shape.x)) / 25 + ((y - shape.y) * (y - shape.y)) / 15 <= 1;
    case ShapeType::Diamond: {
        // ����Ƿ���������
        int dx = abs(x - shape.x);
        int dy = abs(y - shape.y);
        return dx + dy <= 25;
    }
    default:
        return false;
    }
}

