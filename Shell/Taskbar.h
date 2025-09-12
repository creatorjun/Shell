#pragma once
#include <windows.h>
#include <memory>

// GDI+ 클래스 전방 선언
namespace Gdiplus
{
    class Image;
}

// [수정] 작업 표시줄 관련 기능을 클래스로 캡슐화합니다.
class Taskbar
{
public:
    // 기존의 전역 함수들을 public static 멤버 함수로 변경합니다.
    static ATOM Register(HINSTANCE hInstance, const WCHAR* szWindowClass);
    static BOOL Create(HINSTANCE hInst, int nCmdShow,
        const WCHAR* szTitle, const WCHAR* szWindowClass,
        std::unique_ptr<Gdiplus::Image> start);
};