#include "Sidebar.h"

// 构造函数，初始化工具栏并绑定按钮事件
Sidebar::Sidebar(wxFrame* parent, DrawingPanel* drawingPanel)
    : wxPanel(parent, wxID_ANY), drawingPanel(drawingPanel) {
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    wxButton* rectButton = new wxButton(this, wxID_ANY, "Rectangle");
    wxButton* circleButton = new wxButton(this, wxID_ANY, "Circle");
    wxButton* triangleButton = new wxButton(this, wxID_ANY, "Triangle");
    wxButton* ellipseButton = new wxButton(this, wxID_ANY, "Ellipse");
    wxButton* diamondButton = new wxButton(this, wxID_ANY, "Diamond");

    sizer->Add(rectButton, 0, wxALL, 5);
    sizer->Add(circleButton, 0, wxALL, 5);
    sizer->Add(triangleButton, 0, wxALL, 5);
    sizer->Add(ellipseButton, 0, wxALL, 5);
    sizer->Add(diamondButton, 0, wxALL, 5);

    SetSizer(sizer);

    rectButton->Bind(wxEVT_BUTTON, &Sidebar::OnRectButton, this);
    circleButton->Bind(wxEVT_BUTTON, &Sidebar::OnCircleButton, this);
    triangleButton->Bind(wxEVT_BUTTON, &Sidebar::OnTriangleButton, this);
    ellipseButton->Bind(wxEVT_BUTTON, &Sidebar::OnEllipseButton, this);
    diamondButton->Bind(wxEVT_BUTTON, &Sidebar::OnDiamondButton, this);
}

void Sidebar::OnRectButton(wxCommandEvent& event) {
    drawingPanel->AddShape(ShapeType::Rectangle, 50, 50);
}

void Sidebar::OnCircleButton(wxCommandEvent& event) {
    drawingPanel->AddShape(ShapeType::Circle, 50, 50);
}

void Sidebar::OnTriangleButton(wxCommandEvent& event) {
    drawingPanel->AddShape(ShapeType::Triangle, 50, 50);
}

void Sidebar::OnEllipseButton(wxCommandEvent& event) {
    drawingPanel->AddShape(ShapeType::Ellipse, 50, 50);
}

void Sidebar::OnDiamondButton(wxCommandEvent& event) {
    drawingPanel->AddShape(ShapeType::Diamond, 50, 50);
}
