#pragma once
#pragma once
#include <windows.h> // RECT Ÿ���� ����ϱ� ���� ����

// GDI+ Ŭ���� ���� ����
namespace Gdiplus
{
    class Graphics;
    class Image;
}

/**
 * @brief �۾�ǥ���� ���� ���� ��ư �̹����� �׸��ϴ�.
 * @param gfx - �׸��� ����� �۾�ǥ������ GDI+ Graphics ��ü ������
 * @param taskbarRect - �۾�ǥ���� â�� ��ü RECT
 * @param imgStart - ���ҽ����� �ε�� ���� ��ư GDI+ Image ��ü ������
 */
void StartButton_Paint(Gdiplus::Graphics* gfx, RECT taskbarRect, Gdiplus::Image* imgStart);