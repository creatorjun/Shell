// StartButton.cpp : ���� ��ư �׸��� ����� �����մϴ�.
//

// windows.h �� GDI+�� min/max ��ũ�� �浹�� �����մϴ�.
#define NOMINMAX

#include "StartButton.h" // �� ����� ���

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
 */
void StartButton_Paint(Gdiplus::Graphics* gfx, RECT taskbarRect, Gdiplus::Image* imgStart)
{
    // Graphics ��ü�� �̹��� �����Ͱ� ��ȿ���� ������ �ƹ��͵� ���� ����
    if (!gfx || !imgStart)
    {
        return;
    }

    // �۾�ǥ������ ��ü �ʺ�� ���� ���
    int clientWidth = taskbarRect.right - taskbarRect.left;
    int clientHeight = taskbarRect.bottom - taskbarRect.top;

    // ���� ��ư�� 32x32 ũ��(����)�� ����
    const int targetSize = 32;

    // ��� ��ǥ ���: (��ü �ʺ� / 2) - (�̹��� �ʺ� / 2)
    int posX = (clientWidth - targetSize) / 2;
    // ���� �߾� ��ǥ ���: (��ü ���� / 2) - (�̹��� ���� / 2)
    int posY = (clientHeight - targetSize) / 2;


    // --- [����ȭ 1: �̹��� ǰ�� ���� (���� ������)] ---
    // (����) GetInterpolationMode�� ���� ���� ��ȯ�մϴ�.
    InterpolationMode oldMode = gfx->GetInterpolationMode(); // ���� ��� ���
    gfx->SetInterpolationMode(InterpolationModeHighQualityBicubic);
    // --- [���� ��] ---


    // GDI+�� ����Ͽ� �̹��� �׸��� (������¡ ����)
    gfx->DrawImage(imgStart, posX, posY, targetSize, targetSize);


    // --- [����ȭ: �׷��Ƚ� ���� ����] ---
    gfx->SetInterpolationMode(oldMode); // ���� ��带 ������� ����
    // --- [����ȭ ��] ---
}