#ifndef POINTGRID_H
#define POINTGRID_H

#include <wx/wx.h>
#include <wx/dcbuffer.h> // ������ͷ�ļ���ʹ�� wxBufferedPaintDC

// ���� PointGrid �࣬���ڻ������񱳾�
class PointGrid {
public:
    PointGrid(wxPanel* panel, int spacing);
    void Draw(wxBufferedPaintDC& dc);

private:
    wxPanel* panel; // ָ���ͼ����ָ��
    int spacing;    // ����ļ��
};

#endif // POINTGRID_H
