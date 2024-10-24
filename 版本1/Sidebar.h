#pragma once
#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <wx/wx.h>
#include "DrawingPanel.h"

// ���� Sidebar �࣬�̳��� wxPanel��������ʾ������
class Sidebar : public wxPanel {
public:
    Sidebar(wxFrame* parent, DrawingPanel* drawingPanel); // ���캯��

private:
    void OnRectButton(wxCommandEvent& event); // ������ΰ�ť����¼�
    void OnCircleButton(wxCommandEvent& event); // ����Բ�ΰ�ť����¼�
    void OnTriangleButton(wxCommandEvent& event); // ���������ΰ�ť����¼�
    void OnEllipseButton(wxCommandEvent& event); // ������Բ��ť����¼�
    void OnDiamondButton(wxCommandEvent& event); // �������ΰ�ť����¼�

    DrawingPanel* drawingPanel; // ָ���ͼ����ָ��
};

#endif // SIDEBAR_H
