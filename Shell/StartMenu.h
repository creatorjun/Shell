// creatorjun/shell/Shell-04869ccf080a38ea887a3d00139afecc32776daa/Shell/StartMenu.h

#pragma once
#include <windows.h> // HWND, HINSTANCE 등 기본 타입

// --- [커스텀 메시지 정의] ---
// 시작 메뉴가 닫힐 때 Taskbar에 알려주기 위한 사용자 정의 메시지입니다.
#define WM_APP_MENU_CLOSED (WM_APP + 1)


// [수정] 시작 메뉴 관련 기능을 클래스로 캡슐화합니다.
class StartMenu
{
public:
    // 기존의 전역 함수들을 public static 멤버 함수로 변경합니다.
    static ATOM Register(HINSTANCE hInstance);
    static HWND Create(HINSTANCE hInstance, HWND hOwner);
    static void Show(HWND hStartMenu, HWND hTaskbar);
    static void Hide(HWND hStartMenu);
};