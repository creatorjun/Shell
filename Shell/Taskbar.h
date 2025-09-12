#pragma once
#include <windows.h>
#include <memory>

// GDI+ Ŭ���� ���� ����
namespace Gdiplus
{
    class Image;
}

// [����] �۾� ǥ���� ���� ����� Ŭ������ ĸ��ȭ�մϴ�.
class Taskbar
{
public:
    // ������ ���� �Լ����� public static ��� �Լ��� �����մϴ�.
    static ATOM Register(HINSTANCE hInstance, const WCHAR* szWindowClass);
    static BOOL Create(HINSTANCE hInst, int nCmdShow,
        const WCHAR* szTitle, const WCHAR* szWindowClass,
        std::unique_ptr<Gdiplus::Image> start);
};