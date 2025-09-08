// creatorjun/shell/Shell-04869ccf080a38ea887a3d00139afecc32776daa/Shell/StartMenu.h

#pragma once
#include <windows.h> // HWND, HINSTANCE �� �⺻ Ÿ��

// --- [Ŀ���� �޽��� ����] ---
// [�ٽ� �߰�]
// ���� �޴��� ���� �� (��Ŀ���� �Ұų�, ESC�� �����ų�, �׸��� Ŭ���� ��) 
// Taskbar�� �˷��ֱ� ���� ����� ���� �޽����Դϴ�.
#define WM_APP_MENU_CLOSED (WM_APP + 1)


/**
 * @brief ���� �޴� ������ Ŭ������ ����մϴ�.
 * @param hInstance - ���ø����̼� �ν��Ͻ�
 * @return ATOM ������ Ŭ���� Atom
 */
ATOM StartMenu_Register(HINSTANCE hInstance);

/**
 * @brief ���� �޴� �����츦 (������ ���·�) �����մϴ�.
 * @param hInstance - ���ø����̼� �ν��Ͻ�
 * @param hOwner - ������ ������ (Taskbar�� HWND)
 * @return ���� �� ���� �޴��� HWND, ���� �� NULL
 */
HWND StartMenu_Create(HINSTANCE hInstance, HWND hOwner);

/**
 * @brief ���� �޴��� �ִϸ��̼ǰ� �Բ� ȭ�鿡 ǥ���մϴ�.
 * @param hStartMenu - ���� �޴��� HWND
 * @param hTaskbar - �۾�ǥ������ HWND (��ġ ��� ����)
 */
void StartMenu_Show(HWND hStartMenu, HWND hTaskbar);

/**
 * @brief ���� �޴��� �ִϸ��̼ǰ� �Բ� ȭ�鿡�� ����ϴ�.
 * @param hStartMenu - ���� �޴��� HWND
 */
void StartMenu_Hide(HWND hStartMenu);