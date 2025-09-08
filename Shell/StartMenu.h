// creatorjun/shell/Shell-04869ccf080a38ea887a3d00139afecc32776daa/Shell/StartMenu.h

#pragma once
#include <windows.h> // HWND, HINSTANCE 등 기본 타입

// --- [커스텀 메시지 정의] ---
// [다시 추가]
// 시작 메뉴가 닫힐 때 (포커스를 잃거나, ESC를 누르거나, 항목을 클릭할 때) 
// Taskbar에 알려주기 위한 사용자 정의 메시지입니다.
#define WM_APP_MENU_CLOSED (WM_APP + 1)


/**
 * @brief 시작 메뉴 윈도우 클래스를 등록합니다.
 * @param hInstance - 애플리케이션 인스턴스
 * @return ATOM 윈도우 클래스 Atom
 */
ATOM StartMenu_Register(HINSTANCE hInstance);

/**
 * @brief 시작 메뉴 윈도우를 (숨겨진 상태로) 생성합니다.
 * @param hInstance - 애플리케이션 인스턴스
 * @param hOwner - 소유자 윈도우 (Taskbar의 HWND)
 * @return 성공 시 시작 메뉴의 HWND, 실패 시 NULL
 */
HWND StartMenu_Create(HINSTANCE hInstance, HWND hOwner);

/**
 * @brief 시작 메뉴를 애니메이션과 함께 화면에 표시합니다.
 * @param hStartMenu - 시작 메뉴의 HWND
 * @param hTaskbar - 작업표시줄의 HWND (위치 계산 기준)
 */
void StartMenu_Show(HWND hStartMenu, HWND hTaskbar);

/**
 * @brief 시작 메뉴를 애니메이션과 함께 화면에서 숨깁니다.
 * @param hStartMenu - 시작 메뉴의 HWND
 */
void StartMenu_Hide(HWND hStartMenu);