#pragma once
#pragma once
#include <windows.h>

/**
 * @brief 바탕화면 우클릭 메뉴 기능을 캡슐화하는 클래스입니다.
 */
class DesktopMenu
{
public:
    /**
     * @brief 바탕화면 메뉴 창의 윈도우 클래스를 등록합니다.
     * @param hInstance - 애플리케이션 인스턴스 핸들
     * @return 성공 시 RegisterClassExW가 반환하는 ATOM 값, 실패 시 0
     */
    static ATOM Register(HINSTANCE hInstance);

    /**
     * @brief 바탕화면 메뉴 창을 생성합니다.
     * @param hInstance - 애플리케이션 인스턴스 핸들
     * @param hOwner - 부모(소유자) 윈도우 핸들
     * @return 성공 시 생성된 창 핸들(HWND), 실패 시 NULL
     */
    static HWND Create(HINSTANCE hInstance, HWND hOwner);

    /**
     * @brief 지정된 위치에 바탕화면 메뉴 창을 표시합니다.
     * @param hMenu - 표시할 메뉴 창의 핸들
     * @param x - 화면 좌측 상단 기준 x 좌표
     * @param y - 화면 좌측 상단 기준 y 좌표
     */
    static void Show(HWND hMenu, int x, int y);

    /**
     * @brief 바탕화면 메뉴 창을 숨깁니다.
     * @param hMenu - 숨길 메뉴 창의 핸들
     */
    static void Hide(HWND hMenu);
};