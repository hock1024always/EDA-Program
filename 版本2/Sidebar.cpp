#include "Sidebar.h"
#include "DrawingPanel.h"

Sidebar::Sidebar(wxWindow* parent, DrawingPanel* drawingPanel)
    : wxPanel(parent, wxID_ANY) {

    wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);

    // ����һ��������ʾ���Ե�����
    wxPanel* propertiesPanel = new wxPanel(this, wxID_ANY);
    wxBoxSizer* propertiesSizer = new wxBoxSizer(wxVERTICAL);

    componentNameLabel = new wxStaticText(propertiesPanel, wxID_ANY, "Ԫ������: ");
    inputCountLabel = new wxStaticText(propertiesPanel, wxID_ANY, "���������: ");
    outputCountLabel = new wxStaticText(propertiesPanel, wxID_ANY, "���������: ");
    functionLabel = new wxStaticText(propertiesPanel, wxID_ANY, "����: ");

    propertiesSizer->Add(componentNameLabel, 0, wxALL, 5);
    propertiesSizer->Add(inputCountLabel, 0, wxALL, 5);
    propertiesSizer->Add(outputCountLabel, 0, wxALL, 5);
    propertiesSizer->Add(functionLabel, 0, wxALL, 5);

    propertiesPanel->SetSizer(propertiesSizer);
    vbox->Add(propertiesPanel, 0, wxEXPAND | wxALL, 5);

    SetSizer(vbox);
}

void Sidebar::DisplayComponentProperties(const wxString& componentName, int inputCount, int outputCount, const wxString& function) {
    componentNameLabel->SetLabel("Ԫ������: " + componentName);
    inputCountLabel->SetLabel("���������: " + wxString::Format("%d", inputCount));
    outputCountLabel->SetLabel("���������: " + wxString::Format("%d", outputCount));
    functionLabel->SetLabel("����: " + function);
}