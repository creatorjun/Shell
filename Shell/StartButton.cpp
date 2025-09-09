// creatorjun/shell/Shell-04869ccf080a38ea887a3d00139afecc32776daa/Shell/StartButton.cpp

// StartButton.cpp : ���� ��ư �׸��� ����� �����մϴ�.
//

// windows.h �� GDI+�� min/max ��ũ�� �浹�� �����մϴ�.
#define NOMINMAX

#include "framework.h"
#include "StartButton.h"    // �� ����� ���
#include "GraphicsUtil.h"   // �ձ� �簢�� ���� �Լ��� ���� ����
#include "Constants.h"      // [�߰�] ��� ���� ����� �����մϴ�.

// --- GDI+ ���� ---
#include <Objidl.h>          // IStream ���� ���� GDI+���� ���� ����
#include <gdiplus.h>         // GDI+ ���
using namespace Gdiplus;     // GDI+ ���ӽ����̽�
// --- GDI+ �� ---


/**
 * @brief �۾�ǥ���� HDC(Graphics ��ü) ���� ���� ��ư �̹����� �׸��ϴ�.
 * @param gfx - �׸��� ����� �۾�ǥ������ GDI+ Graphics ��ü ������
 * @param taskbarRect - �۾�ǥ���� â�� ��ü RECT (��ǥ ��꿡 ����)
 * @param imgStart - ���ҽ����� �ε�� ���� ��ư GDI+ Image ��ü ������
 * @param bIsHover - ���콺 ȣ�� ���� �÷���
 */
void StartButton_Paint(Gdiplus::Graphics* gfx, RECT taskbarRect, Gdiplus::Image* imgStart, BOOL bIsHover)
{
    if (!gfx || !imgStart)
    {
        return;
    }

    int clientWidth = taskbarRect.right - taskbarRect.left;
    int clientHeight = taskbarRect.bottom - taskbarRect.top;

    // [����] ��� ���
    // 1. ������ ũ��
    int posX = (clientWidth - START_BUTTON_SIZE) / 2;
    int posY = (clientHeight - START_BUTTON_SIZE) / 2;

    // 2. ȣ�� ���� ũ��
    int hoverX = (clientWidth - START_BUTTON_HOVER_SIZE) / 2;
    int hoverY = (clientHeight - START_BUTTON_HOVER_SIZE) / 2;

    if (bIsHover)
    {
        SolidBrush hoverBrush(Color(76, 204, 153, 255));
        float cornerRadius = 0.0f;

        // [����] ��� ���
        RectF hoverRectF((REAL)hoverX, (REAL)hoverY, (REAL)START_BUTTON_HOVER_SIZE, (REAL)START_BUTTON_HOVER_SIZE);

        GraphicsPath path;
        CreateRoundedRectPath(&path, hoverRectF, cornerRadius);
        gfx->FillPath(&hoverBrush, &path);
    }

    InterpolationMode oldMode = gfx->GetInterpolationMode();
    gfx->SetInterpolationMode(InterpolationModeHighQualityBicubic);

    // [����] ��� ���
    gfx->DrawImage(imgStart, posX, posY, START_BUTTON_SIZE, START_BUTTON_SIZE);

    gfx->SetInterpolationMode(oldMode);
}