// creatorjun/shell/Shell-04869ccf080a38ea887a3d00139afecc32776daa/Shell/StartMenu.h

#pragma once
#include <windows.h> // HWND, HINSTANCE �� �⺻ Ÿ��

// --- [Ŀ���� �޽��� ����] ---
// ���� �޴��� ���� �� Taskbar�� �˷��ֱ� ���� ����� ���� �޽����Դϴ�.
#define WM_APP_MENU_CLOSED (WM_APP + 1)


// [����] ���� �޴� ���� ����� Ŭ������ ĸ��ȭ�մϴ�.
class StartMenu
{
public:
    // ������ ���� �Լ����� public static ��� �Լ��� �����մϴ�.
    static ATOM Register(HINSTANCE hInstance);
    static HWND Create(HINSTANCE hInstance, HWND hOwner);
    static void Show(HWND hStartMenu, HWND hTaskbar);
    static void Hide(HWND hStartMenu);
};