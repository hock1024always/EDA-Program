#include <wx/wx.h>
#include "MainFrame.h"

// ���� MyApp �࣬�̳��� wxApp������Ӧ�ó�������
class MyApp : public wxApp {
public:
    // Ӧ�ó����ʼ������
    virtual bool OnInit() {
        MainFrame* frame = new MainFrame(); // ����������
        frame->Show(true);                  // ��ʾ������
        return true;                        // ���� true ��ʾ��ʼ���ɹ�
    }
};

// ʵ��Ӧ�ó������
wxIMPLEMENT_APP(MyApp);