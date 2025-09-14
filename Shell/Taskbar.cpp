// Taskbar.cpp : �۾�ǥ����(AppBar) â ����, ��� �� �޽��� ó���� �����մϴ�.
//

// windows.h �� GDI+�� min/max ��ũ�� �浹�� �����մϴ�.
#define NOMINMAX

#include "framework.h"
#include "Taskbar.h"
#include "StartButton.h"
#include "StartMenu.h"
#include "Shell.h"
#include "Resource.h"
#include "Constants.h"
#include "Clock.h" // [�߰�] Clock ��� ��� ����

#include <shellapi.h>
#include <windowsx.h>
#include <memory>
#include <string>

// --- [GDI+ ����] ---
#include <Objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;
// --- [GDI+ ��] ---

INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

namespace
{
    // --- [���� ������ ����ü] ---
    struct TASKBAR_DATA
    {
        std::unique_ptr<Gdiplus::Image> imgStart;

        BOOL bStartHover;
        BOOL bMouseTracking;
        BOOL bStartActive;
        HWND hStartMenu;

        // [����] �ð� �ؽ�Ʈ ���۸� ����
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

    // �ð� ������Ʈ Ÿ�̸� ID
    const UINT_PTR IDT_CLOCK_TIMER = 1;

    // --- [���� ���� �Լ�] ---
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

    // [����] UpdateClock �Լ� ����

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

        // [����] Clock::Paint �Լ� ȣ��� ����
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

            // [�߰�] Clock ��� �ʱ�ȭ
            if (!Clock::Initialize())
            {
                // �ʱ�ȭ ���� �� ó�� (��: ���� �޽��� ��� �� â ����)
                MessageBox(hWnd, L"�ð� ���ҽ��� �ʱ�ȭ�� �� �����ϴ�.", L"����", MB_OK | MB_ICONERROR);
                return -1; // â ���� ����
            }


            SetTimer(hWnd, IDT_CLOCK_TIMER, 1000, NULL);

            // [����] Clock::FormatTime �Լ� ȣ��� ����
            Clock::FormatTime(pData->szClock, _countof(pData->szClock));
            InvalidateRect(hWnd, NULL, FALSE); // ���� �ð� ǥ�ø� ���� ��ü ����

            break;
        }
        case WM_TIMER:
        {
            if (wParam == IDT_CLOCK_TIMER)
            {
                // [����] Ÿ�̸� �̺�Ʈ �߻� �� Clock ����� ���� �ð� ����
                TASKBAR_DATA* pData = (TASKBAR_DATA*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
                if (pData)
                {
                    Clock::FormatTime(pData->szClock, _countof(pData->szClock));

                    // �ð� ������ ����
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
            // [�߰�] Clock ��� ���ҽ� ����
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

} // �͸� ���ӽ����̽� ��

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