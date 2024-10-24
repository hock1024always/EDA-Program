#include "MainFrame.h"

// ���캯������ʼ�������ڲ����ò���
MainFrame::MainFrame() : wxFrame(nullptr, wxID_ANY, "wxWidgets Drawing App") {
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL); // ����ˮƽ���ֵ� sizer

    drawingPanel = new DrawingPanel(this); // ������ͼ���
    sidebar = new Sidebar(this, drawingPanel); // ����������

    sizer->Add(sidebar, 0, wxEXPAND | wxALL, 5);  // ����������ӵ� sizer
    sizer->Add(drawingPanel, 1, wxEXPAND | wxALL, 5); // ����ͼ�����ӵ� sizer

    SetSizerAndFit(sizer); // ���� sizer ���������ڴ�С
    SetSize(800, 600);     // ���ô��ڴ�С
}