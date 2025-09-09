#pragma once
#include <windows.h> // ATOM, BOOL, HINSTANCE 등 Windows 타입을 사용하기 위해 포함
#include <memory>    // std::unique_ptr 사용을 위해 추가

// GDI+ 클래스 전방 선언
namespace Gdiplus
{
    class Image;
}

/**
 * @brief 작업표시줄(AppBar)의 윈도우 클래스를 시스템에 등록합니다.
 * @param hInstance - 애플리케이션 인스턴스
 * @param szWindowClass - 등록할 창 클래스 이름
 * @return ATOM 윈도우 클래스 Atom
 */
ATOM Taskbar_Register(HINSTANCE hInstance, const WCHAR* szWindowClass);

/**
 * @brief 작업표시줄(AppBar) 윈도우를 생성하고 화면에 표시합니다.
 * @param hInst - 애플리케이션 인스턴스
 * @param nCmdShow - 윈도우 표시 상태
 * @param szTitle - 창 타이틀
 * @param szWindowClass - 등록된 창 클래스 이름
 * @param bg - [수정] 작업표시줄 배경으로 사용할 GDI+ Image 객체의 소유권을 갖는 std::unique_ptr
 * @param start - [수정] 시작 버튼으로 사용할 GDI+ Image 객체의 소유권을 갖는 std::unique_ptr
 * @return 성공 시 TRUE, 실패 시 FALSE
 */
BOOL Taskbar_Create(HINSTANCE hInst, int nCmdShow,
    const WCHAR* szTitle, const WCHAR* szWindowClass,
    std::unique_ptr<Gdiplus::Image> bg, std::unique_ptr<Gdiplus::Image> start);