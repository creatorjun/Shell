// Taskbar.cpp : 작업표시줄(AppBar) 창 생성, 등록 및 메시지 처리를 구현합니다.
//

// windows.h 와 GDI+의 min/max 매크로 충돌을 방지합니다.
#define NOMINMAX

#include "framework.h"
#include "Taskbar.h"
#include "StartButton.h"
#include "StartMenu.h"
#include "Shell.h"
#include "Resource.h"
#include "Constants.h"
#include "Clock.h" // [추가] Clock 모듈 헤더 포함

#include <shellapi.h>
#include <windowsx.h>
#include <memory>
#include <string>

// --- [GDI+ 설정] ---
#include <Objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;
// --- [GDI+ 끝] ---

INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

namespace
{
    // --- [내부 데이터 구조체] ---
    struct TASKBAR_DATA
    {
        std::unique_ptr<Gdiplus::Image> imgStart;

        BOOL bStartHover;
        BOOL bMouseTracking;
        BOOL bStartActive;
        HWND hStartMenu;

        // [수정] 시계 텍스트 버퍼만 남김
        WCHAR szClock[64];


        TASKBAR_DATA() :
            imgStart(nullptr),
            bStartHover(FALSE),
            bMouseTracking(FALSE),
            bStartActive(FALSE),
            hStartMenu(NULL)
        {
            szClock[0] = L'\0';
        }
    };

    static HINSTANCE g_hInstTaskbar;

    // 시계 업데이트 타이머 ID
    const UINT_PTR IDT_CLOCK_TIMER = 1;

    // --- [내부 헬퍼 함수] ---
    void GetStartButtonRect(HWND hWnd, RECT* pButtonRect)
    {
        if (!pButtonRect) return;
        RECT rcClient;
        GetClientRect(hWnd, &rcClient);
        int clientWidth = rcClient.right - rcClient.left;
        int clientHeight = rcClient.bottom - rcClient.top;
        int hoverX = (clientWidth - START_BUTTON_HOVER_SIZE) / 2;
        int hoverY = (clientHeight - START_BUTTON_HOVER_SIZE) / 2;
        SetRect(pButtonRect, hoverX, hoverY, hoverX + START_BUTTON_HOVER_SIZE, hoverY + START_BUTTON_HOVER_SIZE);
    }

    // [제거] UpdateClock 함수 제거

    void UpdateTaskbarAppearance(HWND hWnd)
    {
        TASKBAR_DATA* pData = (TASKBAR_DATA*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if (!pData) return;

        RECT rcClient;
        GetClientRect(hWnd, &rcClient);
        int clientWidth = rcClient.right - rcClient.left;
        int clientHeight = rcClient.bottom - rcClient.top;

        HDC hdcScreen = GetDC(hWnd);
        if (!hdcScreen) return;

        HDC hdcMem = CreateCompatibleDC(hdcScreen);
        if (!hdcMem)
        {
            ReleaseDC(hWnd, hdcScreen);
            return;
        }

        BITMAPINFO bmi = { 0 };
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = clientWidth;
        bmi.bmiHeader.biHeight = -clientHeight; // Top-down DIB
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        void* pvBits;
        HBITMAP hBitmap = CreateDIBSection(hdcMem, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0);

        if (!hBitmap)
        {
            DeleteDC(hdcMem);
            ReleaseDC(hWnd, hdcScreen);
            return;
        }

        HGDIOBJ hOldBitmap = SelectObject(hdcMem, hBitmap);

        Graphics graphics(hdcMem);
        graphics.SetSmoothingMode(SmoothingModeAntiAlias);
        graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
        graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

        SolidBrush solidBrush(Color(128, 0, 0, 0));
        graphics.FillRectangle(&solidBrush, 0, 0, clientWidth, clientHeight);

        Pen highlightPen(Color(80, 255, 255, 255), 1);
        Pen shadowPen(Color(80, 0, 0, 0), 1);
        graphics.DrawLine(&highlightPen, 0, 0, clientWidth, 0);
        graphics.DrawLine(&shadowPen, 0, clientHeight - 1, clientWidth, clientHeight - 1);

        if (pData->imgStart)
        {
            BOOL bShowHighlight = (pData->bStartHover || pData->bStartActive);
            StartButton_Paint(&graphics, rcClient, pData->imgStart.get(), bShowHighlight);
        }

        // [수정] Clock::Paint 함수 호출로 변경
        Clock::Paint(&graphics, rcClient, pData->szClock);

        POINT ptSrc = { 0, 0 };
        SIZE sizeWindow = { clientWidth, clientHeight };
        BLENDFUNCTION blend = { 0 };
        blend.BlendOp = AC_SRC_OVER;
        blend.SourceConstantAlpha = 255;
        blend.AlphaFormat = AC_SRC_ALPHA;

        UpdateLayeredWindow(hWnd, hdcScreen, NULL, &sizeWindow, hdcMem, &ptSrc, 0, &blend, ULW_ALPHA);

        SelectObject(hdcMem, hOldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(hdcMem);
        ReleaseDC(hWnd, hdcScreen);
    }


    LRESULT CALLBACK Taskbar_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (message == WM_APP_MENU_CLOSED)
        {
            TASKBAR_DATA* pData = (TASKBAR_DATA*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
            if (pData && pData->bStartActive)
            {
                pData->bStartActive = FALSE;
                POINT ptCursor;
                GetCursorPos(&ptCursor);
                ScreenToClient(hWnd, &ptCursor);
                RECT rcButton;
                GetStartButtonRect(hWnd, &rcButton);
                BOOL bIsOverButton = PtInRect(&rcButton, ptCursor);
                if (bIsOverButton != pData->bStartHover) pData->bStartHover = bIsOverButton;
                UpdateTaskbarAppearance(hWnd);
            }
            return 0;
        }

        switch (message)
        {
        case WM_CREATE:
        {
            CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
            TASKBAR_DATA* pData = (TASKBAR_DATA*)pCreate->lpCreateParams;
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pData);

            // [추가] Clock 모듈 초기화
            if (!Clock::Initialize())
            {
                // 초기화 실패 시 처리 (예: 에러 메시지 출력 후 창 종료)
                MessageBox(hWnd, L"시계 리소스를 초기화할 수 없습니다.", L"오류", MB_OK | MB_ICONERROR);
                return -1; // 창 생성 실패
            }


            SetTimer(hWnd, IDT_CLOCK_TIMER, 1000, NULL);

            // [수정] Clock::FormatTime 함수 호출로 변경
            Clock::FormatTime(pData->szClock, _countof(pData->szClock));
            InvalidateRect(hWnd, NULL, FALSE); // 최초 시간 표시를 위해 전체 갱신

            break;
        }
        case WM_TIMER:
        {
            if (wParam == IDT_CLOCK_TIMER)
            {
                // [수정] 타이머 이벤트 발생 시 Clock 모듈을 통해 시간 갱신
                TASKBAR_DATA* pData = (TASKBAR_DATA*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
                if (pData)
                {
                    Clock::FormatTime(pData->szClock, _countof(pData->szClock));

                    // 시계 영역만 갱신
                    RECT rcClient;
                    GetClientRect(hWnd, &rcClient);
                    RECT rcClock = rcClient;
                    rcClock.left = rcClock.right - 150;
                    InvalidateRect(hWnd, &rcClock, FALSE);
                }
            }
            break;
        }
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(g_hInstTaskbar, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        }
        case WM_PAINT:
        {
            UpdateTaskbarAppearance(hWnd);
            ValidateRect(hWnd, NULL);
            break;
        }
        case WM_MOUSEMOVE:
        {
            TASKBAR_DATA* pData = (TASKBAR_DATA*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
            if (pData)
            {
                if (!pData->bMouseTracking)
                {
                    TRACKMOUSEEVENT tme = { sizeof(tme) };
                    tme.dwFlags = TME_LEAVE;
                    tme.hwndTrack = hWnd;
                    if (TrackMouseEvent(&tme)) pData->bMouseTracking = TRUE;
                }
                POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                RECT rcButton;
                GetStartButtonRect(hWnd, &rcButton);
                BOOL bIsOverButton = PtInRect(&rcButton, pt);
                if (bIsOverButton != pData->bStartHover)
                {
                    pData->bStartHover = bIsOverButton;
                    if (!pData->bStartActive) UpdateTaskbarAppearance(hWnd);
                }
            }
            break;
        }
        case WM_MOUSELEAVE:
        {
            TASKBAR_DATA* pData = (TASKBAR_DATA*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
            if (pData)
            {
                pData->bMouseTracking = FALSE;
                if (pData->bStartHover)
                {
                    pData->bStartHover = FALSE;
                    if (!pData->bStartActive)
                    {
                        UpdateTaskbarAppearance(hWnd);
                    }
                }
            }
            return 0;
        }
        case WM_LBUTTONDOWN:
        {
            TASKBAR_DATA* pData = (TASKBAR_DATA*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
            if (pData && pData->hStartMenu)
            {
                POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                RECT rcButton;
                GetStartButtonRect(hWnd, &rcButton);
                if (PtInRect(&rcButton, pt))
                {
                    pData->bStartActive = !pData->bStartActive;
                    if (pData->bStartActive) StartMenu::Show(pData->hStartMenu, hWnd);
                    else StartMenu::Hide(pData->hStartMenu);
                    UpdateTaskbarAppearance(hWnd);
                }
                else
                {
                    if (pData->bStartActive)
                    {
                        pData->bStartActive = FALSE;
                        StartMenu::Hide(pData->hStartMenu);
                        UpdateTaskbarAppearance(hWnd);
                    }
                }
            }
            break;
        }
        case WM_RBUTTONDOWN:
        {
            TASKBAR_DATA* pData = (TASKBAR_DATA*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
            if (pData && pData->bStartActive)
            {
                pData->bStartActive = FALSE;
                StartMenu::Hide(pData->hStartMenu);
                UpdateTaskbarAppearance(hWnd);
            }
            break;
        }
        case WM_DESTROY:
        {
            // [추가] Clock 모듈 리소스 해제
            Clock::Shutdown();

            KillTimer(hWnd, IDT_CLOCK_TIMER);
            TASKBAR_DATA* pData = (TASKBAR_DATA*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
            if (pData)
            {
                if (pData->hStartMenu) DestroyWindow(pData->hStartMenu);
                delete pData;
            }
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)NULL);
            PostQuitMessage(0);
            break;
        }
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        return 0;
    }

} // 익명 네임스페이스 끝

ATOM Taskbar::Register(HINSTANCE hInstance, const WCHAR* szWindowClass)
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = Taskbar_WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = sizeof(LONG_PTR);
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SHELL));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    return RegisterClassExW(&wcex);
}

BOOL Taskbar::Create(HINSTANCE hInst, int nCmdShow,
    const WCHAR* szTitle, const WCHAR* szWindowClass,
    std::unique_ptr<Gdiplus::Image> start)
{
    g_hInstTaskbar = hInst;
    TASKBAR_DATA* pData = new TASKBAR_DATA();
    if (!pData) return FALSE;

    pData->imgStart = std::move(start);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    int taskbarY = screenHeight - TASKBAR_HEIGHT;

    HWND hWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED,
        szWindowClass, szTitle, WS_POPUP,
        0, taskbarY, screenWidth, TASKBAR_HEIGHT,
        nullptr, nullptr, hInst, (LPVOID)pData
    );
    if (!hWnd)
    {
        delete pData;
        return FALSE;
    }

    pData->hStartMenu = StartMenu::Create(hInst, hWnd);
    if (!pData->hStartMenu)
    {
        DestroyWindow(hWnd);
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateTaskbarAppearance(hWnd);

    return TRUE;
}