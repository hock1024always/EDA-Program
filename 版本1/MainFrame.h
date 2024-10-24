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
    DrawingPanel* drawingPanel; // ָ���ͼ����ָ��
    Sidebar* sidebar;           // ָ�򹤾�����ָ��
};

#endif // MAINFRAME_H
