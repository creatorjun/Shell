// creatorjun/shell/Shell-04869ccf080a38ea887a3d00139afecc32776daa/Shell/Taskbar.cpp

// Taskbar.cpp : �۾�ǥ����(AppBar) â ����, ��� �� �޽��� ó���� �����մϴ�.
//

// windows.h �� GDI+�� min/max ��ũ�� �浹�� �����մϴ�.
#define NOMINMAX

#include "framework.h"
#include "Taskbar.h"        // �� ����� ���
#include "StartButton.h"    // ���� ��ư �׸��� ��� ����
#include "StartMenu.h"      // [�߰�] ���� �޴� ��� ���
#include "Shell.h"          // ���ҽ� ID (IDM_ABOUT ��)
#include "Resource.h"       // ���ҽ� ID

#include <shellapi.h>       // APPBARDATA, SHAppBarMessage ���� ���� �ʼ�
#include <windowsx.h>       // GET_X_LPARAM, GET_Y_LPARAM ��ũ�� ���
#include <memory>           // [�߰�] std::unique_ptr ����� ���� ����

// --- [GDI+ ����] ---
#include <Objidl.h>          // IStream ���� ���� GDI+���� ���� ����
#include <gdiplus.h>         // GDI+ ���
using namespace Gdiplus;     // GDI+ ���ӽ����̽�
// --- [GDI+ ��] ---


// --- [�ܺ� �Լ� ����] ---
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
// --- [���� ��] ---


static HINSTANCE g_hInstTaskbar;

// --- [����: TASKBAR_DATA ����ü] ---
struct TASKBAR_DATA
{
    std::unique_ptr<Gdiplus::Image> imgBackground;
    std::unique_ptr<Gdiplus::Image> imgStart;

    // ���� ���� ����
    BOOL bStartHover;
    BOOL bMouseTracking;
    BOOL bStartActive;

    HWND hStartMenu;

    // [�߰�] �����ڸ� �����Ͽ� ��� ����� ��� �ʱ�ȭ�մϴ�.
    TASKBAR_DATA() :
        imgBackground(nullptr),
        imgStart(nullptr),
        bStartHover(FALSE),
        bMouseTracking(FALSE),
        bStartActive(FALSE),
        hStartMenu(NULL)
    {
    }
};
// --- [���� ��] ---


// --- [���� ��ư ���� ��� ����] ---
void GetStartButtonRect(HWND hWnd, RECT* pButtonRect)
{
    if (!pButtonRect) return;

    RECT rcClient;
    GetClientRect(hWnd, &rcClient);
    int clientWidth = rcClient.right - rcClient.left;
    int clientHeight = rcClient.bottom - rcClient.top;

    const int hoverSize = 40;
    int hoverX = (clientWidth - hoverSize) / 2;
    int hoverY = (clientHeight - hoverSize) / 2;

    SetRect(pButtonRect, hoverX, hoverY, hoverX + hoverSize, hoverY + hoverSize);
}
// --- [�Լ� ��] ---


/**
 * @brief �۾�ǥ����(AppBar)�� �� ������ ���ν���(WndProc)
 */
LRESULT CALLBACK Taskbar_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    TASKBAR_DATA* pData = (TASKBAR_DATA*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    // --- [�� �ڵ鷯: ��Ŀ�� �𵨿�] ---
    if (message == WM_APP_MENU_CLOSED)
    {
        if (pData && pData->bStartActive)
        {
            pData->bStartActive = FALSE;

            POINT ptCursor;
            GetCursorPos(&ptCursor);
            ScreenToClient(hWnd, &ptCursor);

            RECT rcButton;
            GetStartButtonRect(hWnd, &rcButton);

            BOOL bIsOverButton = PtInRect(&rcButton, ptCursor);
            if (bIsOverButton != pData->bStartHover)
            {
                pData->bStartHover = bIsOverButton;
            }

            InvalidateRect(hWnd, &rcButton, FALSE);
        }
        return 0;
    }
    // --- [�� �ڵ鷯 ��] ---


    switch (message)
    {
    case WM_CREATE:
    {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        TASKBAR_DATA* pInitialData = (TASKBAR_DATA*)pCreate->lpCreateParams;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pInitialData);
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
    }
    break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT rcClient;
        GetClientRect(hWnd, &rcClient);
        int clientWidth = rcClient.right - rcClient.left;
        int clientHeight = rcClient.bottom - rcClient.top;

        Bitmap memBitmap(clientWidth, clientHeight);
        Graphics* memGfx = Graphics::FromImage(&memBitmap);

        if (pData && pData->imgBackground)
        {
            TextureBrush tBrush(pData->imgBackground.get());
            memGfx->FillRectangle(&tBrush, 0, 0, clientWidth, clientHeight);
        }
        else
        {
            Color bgColor;
            bgColor.SetFromCOLORREF(GetSysColor(COLOR_WINDOW));
            SolidBrush fallbackBrush(bgColor);
            memGfx->FillRectangle(&fallbackBrush, 0, 0, clientWidth, clientHeight);
        }

        if (pData && pData->imgStart)
        {
            BOOL bShowHighlight = (pData->bStartHover || pData->bStartActive);
            StartButton_Paint(memGfx, rcClient, pData->imgStart.get(), bShowHighlight);
        }

        Graphics screenGfx(hdc);
        screenGfx.DrawImage(&memBitmap, 0, 0);

        delete memGfx;

        EndPaint(hWnd, &ps);
    }
    break;

    case WM_MOUSEMOVE:
    {
        if (pData)
        {
            if (!pData->bMouseTracking)
            {
                TRACKMOUSEEVENT tme = { sizeof(tme) };
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = hWnd;
                if (TrackMouseEvent(&tme))
                {
                    pData->bMouseTracking = TRUE;
                }
            }

            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            RECT rcButton;
            GetStartButtonRect(hWnd, &rcButton);
            BOOL bIsOverButton = PtInRect(&rcButton, pt);

            if (bIsOverButton != pData->bStartHover)
            {
                pData->bStartHover = bIsOverButton;
                if (!pData->bStartActive)
                {
                    InvalidateRect(hWnd, &rcButton, FALSE);
                }
            }
        }
        break;
    }

    case WM_MOUSELEAVE:
    {
        if (pData)
        {
            pData->bMouseTracking = FALSE;

            if (pData->bStartHover)
            {
                pData->bStartHover = FALSE;
                if (!pData->bStartActive)
                {
                    RECT rcButton;
                    GetStartButtonRect(hWnd, &rcButton);
                    InvalidateRect(hWnd, &rcButton, FALSE);
                }
            }
        }
        return 0;
    }


    case WM_LBUTTONDOWN:
    {
        if (pData && pData->hStartMenu)
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            RECT rcButton;
            GetStartButtonRect(hWnd, &rcButton);

            if (PtInRect(&rcButton, pt))
            {
                pData->bStartActive = !pData->bStartActive;

                if (pData->bStartActive)
                {
                    StartMenu_Show(pData->hStartMenu, hWnd);
                }
                else
                {
                    StartMenu_Hide(pData->hStartMenu);
                }

                InvalidateRect(hWnd, &rcButton, FALSE);
            }
            else
            {
                if (pData->bStartActive)
                {
                    pData->bStartActive = FALSE;
                    StartMenu_Hide(pData->hStartMenu);
                    InvalidateRect(hWnd, &rcButton, FALSE);
                }
            }
        }
        break;
    }

    case WM_RBUTTONDOWN:
    {
        if (pData && pData->bStartActive)
        {
            pData->bStartActive = FALSE;
            StartMenu_Hide(pData->hStartMenu);

            RECT rcButton;
            GetStartButtonRect(hWnd, &rcButton);
            InvalidateRect(hWnd, &rcButton, FALSE);
        }
        break;
    }


    case WM_DESTROY:
    {
        APPBARDATA abd = {};
        abd.cbSize = sizeof(APPBARDATA);
        abd.hWnd = hWnd;
        SHAppBarMessage(ABM_REMOVE, &abd);

        if (pData)
        {
            if (pData->hStartMenu)
            {
                DestroyWindow(pData->hStartMenu);
            }
            delete pData;
        }
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)NULL);

        PostQuitMessage(0);
    }
    break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


/**
 * @brief �۾�ǥ����(AppBar)�� ������ Ŭ������ �ý��ۿ� ����մϴ�.
 */
ATOM Taskbar_Register(HINSTANCE hInstance, const WCHAR* szWindowClass)
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


/**
 * @brief �۾�ǥ����(AppBar) �����츦 �����ϰ� ȭ�鿡 ǥ���մϴ�.
 */
BOOL Taskbar_Create(HINSTANCE hInst, int nCmdShow,
    const WCHAR* szTitle, const WCHAR* szWindowClass,
    std::unique_ptr<Gdiplus::Image> bg, std::unique_ptr<Gdiplus::Image> start)
{
    g_hInstTaskbar = hInst;

    // --- [����: RAII �� ������ Ȱ��] ---
    // 1. new TASKBAR_DATA()�� ȣ���ϸ� ���� ������ �����ڰ� ��� ����� �ʱ�ȭ�մϴ�.
    TASKBAR_DATA* pData = new TASKBAR_DATA();
    if (!pData) return FALSE;

    // 2. unique_ptr�� �������� pData�� ����� �̵�(move)
    pData->imgBackground = std::move(bg);
    pData->imgStart = std::move(start);

    // 3. [����] ���� �ʱ�ȭ �ڵ尡 �� �̻� �ʿ� �����ϴ�.
    // pData->bStartHover = FALSE;
    // pData->bMouseTracking = FALSE;
    // pData->bStartActive = FALSE;
    // pData->hStartMenu = NULL;
    // --- [���� ��] ---

    int barHeight = 48;
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    HWND hWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        szWindowClass,
        szTitle,
        WS_POPUP,
        0, 0,
        screenWidth, barHeight,
        nullptr,
        nullptr,
        hInst,
        (LPVOID)pData
    );

    if (!hWnd)
    {
        delete pData;
        return FALSE;
    }

    pData->hStartMenu = StartMenu_Create(hInst, hWnd);
    if (!pData->hStartMenu)
    {
        DestroyWindow(hWnd);
        return FALSE;
    }

    APPBARDATA abd = {};
    abd.cbSize = sizeof(APPBARDATA);
    abd.hWnd = hWnd;

    if (!SHAppBarMessage(ABM_NEW, &abd))
    {
        DestroyWindow(hWnd);
        return FALSE;
    }

    abd.uEdge = ABE_BOTTOM;
    abd.rc.left = 0;
    abd.rc.right = screenWidth;
    abd.rc.top = screenHeight - barHeight;
    abd.rc.bottom = screenHeight;

    SHAppBarMessage(ABM_QUERYPOS, &abd);

    MoveWindow(hWnd, abd.rc.left, abd.rc.top, abd.rc.right - abd.rc.left, abd.rc.bottom - abd.rc.top, TRUE);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}