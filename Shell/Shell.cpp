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


// [추가] 사용 가능한 최대 해상도로 디스플레이 설정을 변경하는 함수
BOOL SetMaxDisplayResolution()
{
    DEVMODEW dm = { 0 };
    dm.dmSize = sizeof(dm);

    DEVMODEW maxResDm = { 0 };
    maxResDm.dmSize = sizeof(dm);

    int iModeNum = 0;
    // 현재 디스플레이에서 사용 가능한 모든 그래픽 모드를 열거합니다.
    while (EnumDisplaySettingsW(NULL, iModeNum, &dm) != 0)
    {
        // 더 높은 너비의 해상도를 찾거나, 너비가 같다면 더 높은 높이의 해상도를 찾습니다.
        if (dm.dmPelsWidth > maxResDm.dmPelsWidth ||
            (dm.dmPelsWidth == maxResDm.dmPelsWidth && dm.dmPelsHeight > maxResDm.dmPelsHeight))
        {
            maxResDm = dm;
        }
        iModeNum++;
    }

    // 유효한 최대 해상도 모드를 찾았는지 확인합니다.
    if (maxResDm.dmPelsWidth > 0 && maxResDm.dmPelsHeight > 0)
    {
        // 현재 디스플레이 설정을 가져옵니다.
        DEVMODEW currentDm = { 0 };
        currentDm.dmSize = sizeof(currentDm);
        if (EnumDisplaySettingsW(NULL, ENUM_CURRENT_SETTINGS, &currentDm))
        {
            // 현재 설정과 최대 해상도가 다른 경우에만 변경을 시도합니다.
            if (currentDm.dmPelsWidth != maxResDm.dmPelsWidth || currentDm.dmPelsHeight != maxResDm.dmPelsHeight)
            {
                // 해상도 필드만 최대 해상도 값으로 변경합니다.
                currentDm.dmPelsWidth = maxResDm.dmPelsWidth;
                currentDm.dmPelsHeight = maxResDm.dmPelsHeight;
                currentDm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

                // 디스플레이 설정을 변경합니다.
                if (ChangeDisplaySettingsW(&currentDm, CDS_UPDATEREGISTRY) == DISP_CHANGE_SUCCESSFUL)
                {
                    return TRUE; // 성공
                }
            }
            else
            {
                return TRUE; // 이미 최대 해상도이므로 성공으로 간주
            }
        }
    }

    return FALSE; // 실패
}


// 정보 대화 상자의 메시지 처리기입니다.
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

    // [추가] 프로그램 시작 시 가장 먼저 최대 해상도로 설정합니다.
    SetMaxDisplayResolution();

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

    Taskbar::Register(hInstance, szWindowClass);
    StartMenu::Register(hInstance);


    if (!Taskbar::Create(hInstance, nCmdShow, szTitle, szWindowClass,
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