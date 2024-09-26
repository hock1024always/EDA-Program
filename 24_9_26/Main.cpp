#include <wx/wx.h>

class App : public wxApp {
public:
    bool OnInit() {
        wxFrame* window = new wxFrame(NULL, wxID_ANY, "这是一个全新的窗口程序", wxDefaultPosition, wxSize(600, 400));
        wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL); 


        wxButton* button = new wxButton(window, wxID_ANY, "请点击", wxDefaultPosition, wxSize(100, 100));
        button->SetFont(wxFont(20, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

        wxStaticText* counter = new wxStaticText(window, wxID_ANY, "计数: 0", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
        counter->SetFont(wxFont(20, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

        mainSizer->Add(button, 1, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);

        wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
        bottomSizer->AddStretchSpacer(1); 
        bottomSizer->Add(counter, 0, wxALIGN_RIGHT | wxALL, 10); 

        mainSizer->Add(bottomSizer, 0, wxEXPAND | wxALIGN_BOTTOM);

        window->SetSizer(mainSizer);
        window->Show();

        button->Bind(wxEVT_BUTTON, [counter](wxCommandEvent&) {
            static int count = 0;
            count++;
            counter->SetLabel(wxString::Format("计数: %d", count));
            });

        return true;
    }
};

wxIMPLEMENT_APP(App);