#pragma once

// GDI+�� Image Ŭ������ ���� �����մϴ�.
// (��� ���Ͽ� GDI+ ��ü�� �������� �ʱ� ����)
namespace Gdiplus
{
    class Image;
}

/**
 * @brief ����ȭ�� ���ȭ���� �����մϴ�.
 * @param imgBackground - ���ҽ����� �ε�� ���ȭ�� GDI+ Image ��ü ������
 */
void Desktop_SetWallpaper(Gdiplus::Image* imgBackground);