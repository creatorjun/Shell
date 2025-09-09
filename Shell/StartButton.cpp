// creatorjun/shell/Shell-04869ccf080a38ea887a3d00139afecc32776daa/Shell/StartButton.cpp

// StartButton.cpp : ���� ��ư �׸��� ����� �����մϴ�.
//

// windows.h �� GDI+�� min/max ��ũ�� �浹�� �����մϴ�.
#define NOMINMAX

#include "framework.h"      // [�߰�] �̸� �����ϵ� ����� ���� �ֻ�ܿ� ����
#include "StartButton.h"    // �� ����� ���
#include "GraphicsUtil.h"   // [�߰�] �ձ� �簢�� ���� �Լ��� ���� ����

// --- GDI+ ���� ---
#include <Objidl.h>          // IStream ���� ���� GDI+���� ���� ����
#include <gdiplus.h>         // GDI+ ���
using namespace Gdiplus;     // GDI+ ���ӽ����̽�
// --- GDI+ �� ---


// --- [����] ---
// �ߺ��Ǵ� CreateRoundedRectPath �Լ��� GraphicsUtil.cpp�� �̵������Ƿ� �����մϴ�.
// --- [���� ��] ---


/**
 * @brief �۾�ǥ���� HDC(Graphics ��ü) ���� ���� ��ư �̹����� �׸��ϴ�.
 * @param gfx - �׸��� ����� �۾�ǥ������ GDI+ Graphics ��ü ������
 * @param taskbarRect - �۾�ǥ���� â�� ��ü RECT (��ǥ ��꿡 ����)
 * @param imgStart - ���ҽ����� �ε�� ���� ��ư GDI+ Image ��ü ������
 * @param bIsHover - [����] ���콺 ȣ�� ���� �÷���
 */
void StartButton_Paint(Gdiplus::Graphics* gfx, RECT taskbarRect, Gdiplus::Image* imgStart, BOOL bIsHover)
{
    // Graphics ��ü�� �̹��� �����Ͱ� ��ȿ���� ������ �ƹ��͵� ���� ����
    if (!gfx || !imgStart)
    {
        return;
    }

    // �۾�ǥ������ ��ü �ʺ�� ���� ���
    int clientWidth = taskbarRect.right - taskbarRect.left;
    int clientHeight = taskbarRect.bottom - taskbarRect.top;

    // --- [����: ȣ�� �� ������ ���� ����] ---

    // 1. ������ ũ�� (����)
    const int targetSize = 32;
    int posX = (clientWidth - targetSize) / 2;
    int posY = (clientHeight - targetSize) / 2;

    // 2. ȣ�� ���� ũ�� (�����ܺ��� �ణ ū 40x40)
    const int hoverSize = 40;
    int hoverX = (clientWidth - hoverSize) / 2;
    int hoverY = (clientHeight - hoverSize) / 2;

    // --- [�� �ڵ�: ȣ�� ���� �׸��� (����� ���� ����/�ݰ� ���)] ---
    if (bIsHover)
    {
        // 30% ������ �ϴû� (Alpha: 76, R: 204, G: 153, B: 255)
        SolidBrush hoverBrush(Color(76, 204, 153, 255));
        float cornerRadius = 0.0f; // ���� �簢��

        // GDI+�� RectF ����
        RectF hoverRectF((REAL)hoverX, (REAL)hoverY, (REAL)hoverSize, (REAL)hoverSize);

        // �ձ� �簢�� ��� ���� (���� GraphicsUtil.h�� �Լ��� ȣ��)
        GraphicsPath path;
        CreateRoundedRectPath(&path, hoverRectF, cornerRadius);

        // �������� �׸��� *����* ��θ� ä���� ��� ���� ȿ���� �ݴϴ�.
        gfx->FillPath(&hoverBrush, &path);
    }
    // --- [�ڵ� ��] ---


    // --- [����ȭ 1: �̹��� ǰ�� ����] ---
    InterpolationMode oldMode = gfx->GetInterpolationMode(); // ���� ��� ���
    gfx->SetInterpolationMode(InterpolationModeHighQualityBicubic);
    // --- ---


    // GDI+�� ����Ͽ� �̹��� �׸��� (������¡ ����)
    gfx->DrawImage(imgStart, posX, posY, targetSize, targetSize);


    // --- [����ȭ: �׷��Ƚ� ���� ����] ---
    gfx->SetInterpolationMode(oldMode); // ���� ��带 ������� ����
    // --- [����ȭ ��] ---
}