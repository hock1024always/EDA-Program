#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <wx/wx.h>

class DrawingPanel; // Ç°ÖÃÉùÃ÷

class Sidebar : public wxPanel {
public:
    Sidebar(wxWindow* parent, DrawingPanel* drawingPanel);

    void DisplayComponentProperties(const wxString& componentName, int inputCount, int outputCount, const wxString& function);

private:
    wxStaticText* componentNameLabel;
    wxStaticText* inputCountLabel;
    wxStaticText* outputCountLabel;
    wxStaticText* functionLabel;
};

#endif // SIDEBAR_H