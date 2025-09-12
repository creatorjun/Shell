#pragma once

#include <Windows.h>

class DesktopContextMenu
{
public:
    DesktopContextMenu();
    ~DesktopContextMenu();

    void Show(HWND hWnd, int x, int y);

private:
    HMENU m_hMenu;
};