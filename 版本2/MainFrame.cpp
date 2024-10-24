#include "MainFrame.h"

// �¼���
wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_BUTTON(wxID_ANY, MainFrame::OnButtonClick)
wxEND_EVENT_TABLE()

// ���캯������ʼ�������ڲ����ò���
MainFrame::MainFrame() : wxFrame(nullptr, wxID_ANY, "wxWidgets Drawing App") {
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL); // ����ˮƽ���ֵ� sizer

    drawingPanel = new DrawingPanel(this); // ������ͼ���
    sidebar = new Sidebar(this, drawingPanel); // ����������

    // ��ȡԭ�Ȳ�����Ĵ�С
    int originalWidth = 150; // ����ԭ�ȵĿ��Ϊ 150 ����
    //int newWidth = 3 * originalWidth; // �µĿ��Ϊԭ�ȵ�����

    sidebar->SetMinSize(wxSize(originalWidth, -1)); // ���ò��������С���Ϊ�¿��

    sizer->Add(sidebar, 0, wxEXPAND | wxALL, 5);  // ����������ӵ� sizer
    sizer->Add(drawingPanel, 1, wxEXPAND | wxALL, 5); // ����ͼ�����ӵ� sizer

    SetSizerAndFit(sizer); // ���� sizer ���������ڴ�С
    SetSize(800, 600);     // ���ô��ڴ�С

    // ������һ����ť���ڲ���
    wxButton* testButton = new wxButton(drawingPanel, wxID_ANY, "Test Button");
    drawingPanel->GetSizer()->Add(testButton, 0, wxALL, 5);
}

void MainFrame::OnButtonClick(wxCommandEvent& event) {
    // ��������ťʱ��ʾ "AddGate" ������
    sidebar->DisplayComponentProperties("AddGate", 2, 1, "˫��Ϊ��");
}

