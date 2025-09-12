#include "DesktopContextMenu.h"
#include "Resource.h"

DesktopContextMenu::DesktopContextMenu()
{
    // �˾� �޴��� �����մϴ�.
    m_hMenu = CreatePopupMenu();
    if (m_hMenu)
    {
        // �޴� �׸���� �߰��մϴ�.
        AppendMenuW(m_hMenu, MF_STRING, ID_DESKTOP_CONTEXT_RESOLUTION, L"�ػ�");
        AppendMenuW(m_hMenu, MF_STRING, ID_DESKTOP_CONTEXT_PROPERTIES, L"�Ӽ�");
        AppendMenuW(m_hMenu, MF_STRING, ID_DESKTOP_CONTEXT_SETTINGS, L"����");
    }
}

DesktopContextMenu::~DesktopContextMenu()
{
    // �޴��� �����Ǿ����� �ı��մϴ�.
    if (m_hMenu)
    {
        DestroyMenu(m_hMenu);
    }
}

void DesktopContextMenu::Show(HWND hWnd, int x, int y)
{
    if (m_hMenu)
    {
        // �޴��� ������ ��ġ�� ǥ���մϴ�.
        // ����ڰ� �޴� ���� Ŭ���ϸ� �ڵ����� �����ϴ�.
        TrackPopupMenu(m_hMenu, TPM_RIGHTBUTTON, x, y, 0, hWnd, NULL);
    }
}