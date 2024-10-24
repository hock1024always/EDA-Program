#pragma once
#ifndef MAINFRAME_H
#define MAINFRAME_H

#include <wx/wx.h>
#include "DrawingPanel.h"
#include "Sidebar.h"

// ���� MainFrame �࣬�̳��� wxFrame�����ڴ���������
class MainFrame : public wxFrame {
public:
    MainFrame(); // ���캯��

private:
    void OnButtonClick(wxCommandEvent& event); // ��ť����¼�������

    DrawingPanel* drawingPanel; // ָ���ͼ����ָ��
    Sidebar* sidebar;           // ָ�򹤾�����ָ��

    wxDECLARE_EVENT_TABLE();
};

#endif // MAINFRAME_H
