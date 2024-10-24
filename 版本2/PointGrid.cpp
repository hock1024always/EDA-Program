#include "PointGrid.h"

// ���캯������ʼ�����񱳾������ͼ��
PointGrid::PointGrid(wxPanel* panel, int spacing) : panel(panel), spacing(spacing) {}

// �������񱳾�
void PointGrid::Draw(wxBufferedPaintDC& dc) {
    // ��������ı�����ɫ
    wxBrush brush(wxColour(220, 226, 241)); // ������ɫ
    dc.SetBrush(brush);
    dc.DrawRectangle(0, 0, panel->GetSize().GetWidth(), panel->GetSize().GetHeight()); // ���Ʊ���

    // ����������ɫ�Ϳ��
    wxPen pen(wxColour(255, 242, 226), 1); // ������ɫ�Ϳ��
    dc.SetPen(pen);

    // ����������
    for (int x = 0; x < panel->GetSize().GetWidth(); x += spacing) {
        dc.DrawLine(x, 0, x, panel->GetSize().GetHeight()); // ��ֱ��
    }
    for (int y = 0; y < panel->GetSize().GetHeight(); y += spacing) {
        dc.DrawLine(0, y, panel->GetSize().GetWidth(), y); // ˮƽ��
    }
}