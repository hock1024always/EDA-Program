#include "MainFrame.h"

// 事件表
wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_BUTTON(wxID_ANY, MainFrame::OnButtonClick)
wxEND_EVENT_TABLE()

// 构造函数，初始化主窗口并设置布局
MainFrame::MainFrame() : wxFrame(nullptr, wxID_ANY, "wxWidgets Drawing App") {
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL); // 创建水平布局的 sizer

    drawingPanel = new DrawingPanel(this); // 创建绘图面板
    sidebar = new Sidebar(this, drawingPanel); // 创建工具栏

    // 获取原先侧边栏的大小
    int originalWidth = 150; // 假设原先的宽度为 150 像素
    //int newWidth = 3 * originalWidth; // 新的宽度为原先的三倍

    sidebar->SetMinSize(wxSize(originalWidth, -1)); // 设置侧边栏的最小宽度为新宽度

    sizer->Add(sidebar, 0, wxEXPAND | wxALL, 5);  // 将工具栏添加到 sizer
    sizer->Add(drawingPanel, 1, wxEXPAND | wxALL, 5); // 将绘图面板添加到 sizer

    SetSizerAndFit(sizer); // 设置 sizer 并调整窗口大小
    SetSize(800, 600);     // 设置窗口大小

    // 假设有一个按钮用于测试
    wxButton* testButton = new wxButton(drawingPanel, wxID_ANY, "Test Button");
    drawingPanel->GetSizer()->Add(testButton, 0, wxALL, 5);
}

void MainFrame::OnButtonClick(wxCommandEvent& event) {
    // 假设点击按钮时显示 "AddGate" 的属性
    sidebar->DisplayComponentProperties("AddGate", 2, 1, "双真为真");
}

