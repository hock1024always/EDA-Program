#include "Sidebar.h"
#include "DrawingPanel.h"

Sidebar::Sidebar(wxWindow* parent, DrawingPanel* drawingPanel)
    : wxPanel(parent, wxID_ANY) {

    wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);

    // 创建一个用于显示属性的区域
    wxPanel* propertiesPanel = new wxPanel(this, wxID_ANY);
    wxBoxSizer* propertiesSizer = new wxBoxSizer(wxVERTICAL);

    componentNameLabel = new wxStaticText(propertiesPanel, wxID_ANY, "元件名称: ");
    inputCountLabel = new wxStaticText(propertiesPanel, wxID_ANY, "接入口数量: ");
    outputCountLabel = new wxStaticText(propertiesPanel, wxID_ANY, "输出口数量: ");
    functionLabel = new wxStaticText(propertiesPanel, wxID_ANY, "功能: ");

    propertiesSizer->Add(componentNameLabel, 0, wxALL, 5);
    propertiesSizer->Add(inputCountLabel, 0, wxALL, 5);
    propertiesSizer->Add(outputCountLabel, 0, wxALL, 5);
    propertiesSizer->Add(functionLabel, 0, wxALL, 5);

    propertiesPanel->SetSizer(propertiesSizer);
    vbox->Add(propertiesPanel, 0, wxEXPAND | wxALL, 5);

    SetSizer(vbox);
}

void Sidebar::DisplayComponentProperties(const wxString& componentName, int inputCount, int outputCount, const wxString& function) {
    componentNameLabel->SetLabel("元件名称: " + componentName);
    inputCountLabel->SetLabel("接入口数量: " + wxString::Format("%d", inputCount));
    outputCountLabel->SetLabel("输出口数量: " + wxString::Format("%d", outputCount));
    functionLabel->SetLabel("功能: " + function);
}