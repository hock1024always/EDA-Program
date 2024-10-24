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

    pointGrid.Draw(dc); // ��������ͼ

    for (const auto& shape : shapes) {
        switch (shape.type) {
        case ShapeType::OnPin:
            dc.DrawRectangle(shape.x - 10, shape.y - 10, 20, 20); // ������
            break;
        case ShapeType::OffPin:
            dc.DrawCircle(shape.x, shape.y, 5); // Բ��
            break;
        case ShapeType::NotGate: {
            wxPoint points[3] = {
                wxPoint(shape.x, shape.y - 10),
                wxPoint(shape.x - 10, shape.y + 10),
                wxPoint(shape.x + 10, shape.y + 10)
            };
            dc.DrawPolygon(3, points); // ����������
            break;
        }
        case ShapeType::OrGate: {
            dc.DrawEllipse(shape.x - 20, shape.y - 20, 40, 40); // ��Բ�λ���
            break;
        }
        case ShapeType::AndGate: {
            dc.DrawRectangle(shape.x - 20, shape.y - 20, 40, 40); // ������
            break;
        }
        }
    }

    // ��������
    wxPen linePen(wxColour(0, 0, 0), 2); // ����������ɫΪ��ɫ
    dc.SetPen(linePen);
    for (const auto& line : lines) {
        dc.DrawLine(line.startX, line.startY, line.endX, line.endY);
    }

    // ������ڻ������������Ƶ�ǰ����
    if (currentLine.startX != currentLine.endX || currentLine.startY != currentLine.endY) {
        dc.DrawLine(currentLine.startX, currentLine.startY, currentLine.endX, currentLine.endY);
    }
}

void DrawingPanel::OnLeftDown(wxMouseEvent& event) {
    int mouseX = event.GetX();
    int mouseY = event.GetY();
    SnapToPoint(mouseX, mouseY); // ��������뵽��״ͼ

    // ����Ƿ����� pin Ԫ��
    for (size_t i = 0; i < shapes.size(); ++i) {
        if (IsPointInShape(mouseX, mouseY, shapes[i])) {
            // ��׽����һ�� pin Ԫ������ʼ��������
            currentLine.startX = shapes[i].x + 10; // Pin ��ê��λ��
            currentLine.startY = shapes[i].y; // Pin ��ê��λ��
            currentLine.endX = mouseX; // ��ʼ�����Ľ���λ��
            currentLine.endY = mouseY;
            return;
        }
    }
}

void DrawingPanel::OnLeftUp(wxMouseEvent& event) {
    if (currentLine.startX == currentLine.endX && currentLine.startY == currentLine.endY) {
        // �������û���ƶ����򲻴���
        return;
    }

    currentLine.endX = event.GetX();
    currentLine.endY = event.GetY();
    SnapToPoint(currentLine.endX, currentLine.endY); // ���뵽��״ͼ
    lines.push_back(currentLine); // ��¼����
    currentLine.startX = currentLine.endX; // ��������
    currentLine.startY = currentLine.endY;
    Refresh();
}

void DrawingPanel::OnMotion(wxMouseEvent& event) {
    if (event.Dragging()) {
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

//�ж����λ���Ƿ���ͼ����
bool DrawingPanel::IsPointInShape(int x, int y, const Shape& shape) {
    switch (shape.type) {
    case ShapeType::OnPin:
        return x >= shape.x - 10 && x <= shape.x + 10 && y >= shape.y - 10 && y <= shape.y + 10; // �������������
    case ShapeType::OffPin:
        return (x - shape.x) * (x - shape.x) + (y - shape.y) * (y - shape.y) <= 5 * 5; // �����Բ����
    case ShapeType::NotGate: {
        // ���ж��������ڲ����߼�
        return false; // ��Ҫ����ʵ��
    }
    case ShapeType::OrGate: {
        // ����Բ���ڵ��߼�����ʵ��
        return false; // ��Ҫ����ʵ��
    }
    case ShapeType::AndGate:
        return x >= shape.x - 20 && x <= shape.x + 20 && y >= shape.y - 20 && y <= shape.y + 20; // �������������
    default:
        return false;
    }
}



