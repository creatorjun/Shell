// Shell.cpp : 애플리케이션 주 진입점을 정의합니다.
//

// windows.h 와 GDI+의 min/max 매크로 충돌을 방지합니다.
#define NOMINMAX

#include "framework.h"
#include "Shell.h"
#include "Taskbar.h"
#include "Desktop.h"
#include "StartMenu.h"
#include <memory>

// --- [GDI+ 설정] ---
#include <Objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")
// --- [GDI+ 끝] ---


#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];


// [추가] 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // --- [GDI+ 초기화] ---
    ULONG_PTR gdiplusToken;
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    // --- [초기화 끝] ---


    std::unique_ptr<Image> imgTaskbarBg(Image::FromFile(L"Background.jpg"));
    std::unique_ptr<Image> imgStart(Image::FromFile(L"Start.png"));
    std::unique_ptr<Image> imgDesktopBg(Image::FromFile(L"Background.jpg"));


    if (imgDesktopBg)
    {
        Desktop_SetWallpaper(imgDesktopBg.get());
    }

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SHELL, szWindowClass, MAX_LOADSTRING);

    Taskbar_Register(hInstance, szWindowClass);
    StartMenu_Register(hInstance);


    if (!Taskbar_Create(hInstance, nCmdShow, szTitle, szWindowClass,
        std::move(imgTaskbarBg), std::move(imgStart)))
    {
        GdiplusShutdown(gdiplusToken);
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SHELL));

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // --- [GDI+ 종료] ---
    GdiplusShutdown(gdiplusToken);
    // --- [종료 끝] ---

    return (int)msg.wParam;
}