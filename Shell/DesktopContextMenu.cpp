#include "DesktopContextMenu.h"
#include "Resource.h"

DesktopContextMenu::DesktopContextMenu()
{
    // 팝업 메뉴를 생성합니다.
    m_hMenu = CreatePopupMenu();
    if (m_hMenu)
    {
        // 메뉴 항목들을 추가합니다.
        AppendMenuW(m_hMenu, MF_STRING, ID_DESKTOP_CONTEXT_RESOLUTION, L"해상도");
        AppendMenuW(m_hMenu, MF_STRING, ID_DESKTOP_CONTEXT_PROPERTIES, L"속성");
        AppendMenuW(m_hMenu, MF_STRING, ID_DESKTOP_CONTEXT_SETTINGS, L"설정");
    }
}

DesktopContextMenu::~DesktopContextMenu()
{
    // 메뉴가 생성되었으면 파괴합니다.
    if (m_hMenu)
    {
        DestroyMenu(m_hMenu);
    }
}

void DesktopContextMenu::Show(HWND hWnd, int x, int y)
{
    if (m_hMenu)
    {
        // 메뉴를 지정된 위치에 표시합니다.
        // 사용자가 메뉴 밖을 클릭하면 자동으로 닫힙니다.
        TrackPopupMenu(m_hMenu, TPM_RIGHTBUTTON, x, y, 0, hWnd, NULL);
    }
}