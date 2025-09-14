#pragma once
#include <windows.h>

// GDI+ Ŭ���� ���� ����
namespace Gdiplus
{
    class Graphics;
}

namespace Clock
{
    /**
     * @brief �ð� ǥ�ÿ� �ʿ��� GDI+ ���ҽ�(��Ʈ, �귯��)�� �ʱ�ȭ�մϴ�.
     * @return ���� �� TRUE, ���� �� FALSE
     */
    BOOL Initialize();

    /**
     * @brief �ʱ�ȭ�ߴ� GDI+ ���ҽ��� �����մϴ�.
     */
    void Shutdown();

    /**
     * @brief ���� �ð��� ��¥�� ���Ŀ� �´� ���ڿ��� �����մϴ�.
     * @param pszBuffer [out] ���˵� ���ڿ��� ������ ����
     * @param cchBuffer [in] ������ ũ�� (���� ����)
     */
    void FormatTime(WCHAR* pszBuffer, size_t cchBuffer);

    /**
     * @brief ������ Graphics ��ü ���� �ð� ���ڿ��� �׸��ϴ�.
     * @param pGfx - �׸��� �׸� GDI+ Graphics ��ü ������
     * @param rcClient - �׸����� ������ �� Ŭ���̾�Ʈ ���� RECT
     * @param pszTime - ȭ�鿡 ǥ���� �ð� ���ڿ�
     */
    void Paint(Gdiplus::Graphics* pGfx, const RECT& rcClient, const WCHAR* pszTime);
}