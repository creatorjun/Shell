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
    Gdiplus::Image* imgBackground;
    Gdiplus::Image* imgStart;

    // ���� ���� ����
    BOOL bStartHover;       // ���콺�� ���� ��ư ���� �ִ��� ����
    BOOL bMouseTracking;    // WM_MOUSELEAVE �̺�Ʈ�� ���� ������ ����
    BOOL bStartActive;      // ���� ��ư�� Ȱ��ȭ(Ŭ��)�Ǿ����� ����

    HWND hStartMenu;        // ������ ���� �޴� ������ �ڵ�
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
    // (StartMenu�� WM_KILLFOCUS, ESC, ������ Ŭ�� ������ ���� �� �� �޽����� ����)
    if (message == WM_APP_MENU_CLOSED)
    {
        if (pData && pData->bStartActive) // 1. Ȱ�� ������ ���� ó��
        {
            pData->bStartActive = FALSE; // 2. Ȱ�� ���� ����

            // 3. (�� WM_CAPTURECHANGED ������ ����)
            //    �޴��� ���� ������ ���콺�� ��ư ���� �ִ��� Ȯ���Ͽ� ȣ�� ���� ����
            POINT ptCursor;
            GetCursorPos(&ptCursor);        // ȭ��(Screen) ��ǥ
            ScreenToClient(hWnd, &ptCursor); // �۾�ǥ����(Client) ��ǥ�� ��ȯ

            RECT rcButton;
            GetStartButtonRect(hWnd, &rcButton);

            BOOL bIsOverButton = PtInRect(&rcButton, ptCursor);
            if (bIsOverButton != pData->bStartHover)
            {
                pData->bStartHover = bIsOverButton; // ȣ�� ���� ����
            }

            // 4. ���� ���·� ��ư �ٽ� �׸���
            InvalidateRect(hWnd, &rcButton, FALSE);
        }
        return 0; // �޽��� ó�� �Ϸ�
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

    case WM_PAINT: // (���� ����)
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT rcClient;
        GetClientRect(hWnd, &rcClient);
        int clientWidth = rcClient.right - rcClient.left;
        int clientHeight = rcClient.bottom - rcClient.top;

        Bitmap memBitmap(clientWidth, clientHeight);
        Graphics* memGfx = Graphics::FromImage(&memBitmap);

        // ��� �׸���
        if (pData && pData->imgBackground)
        {
            TextureBrush tBrush(pData->imgBackground);
            memGfx->FillRectangle(&tBrush, 0, 0, clientWidth, clientHeight);
        }
        else
        {
            Color bgColor;
            bgColor.SetFromCOLORREF(GetSysColor(COLOR_WINDOW));
            SolidBrush fallbackBrush(bgColor);
            memGfx->FillRectangle(&fallbackBrush, 0, 0, clientWidth, clientHeight);
        }

        // ���� ��ư �׸���
        if (pData && pData->imgStart)
        {
            BOOL bShowHighlight = (pData->bStartHover || pData->bStartActive);
            StartButton_Paint(memGfx, rcClient, pData->imgStart, bShowHighlight);
        }

        Graphics screenGfx(hdc);
        screenGfx.DrawImage(&memBitmap, 0, 0);

        delete memGfx;

        EndPaint(hWnd, &ps);
    }
    break;

    // --- [���콺 ���� ���� (���� ����)] ---
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
    // --- [���콺 ���� ��] ---


    // --- [����: SetCapture ���� ���� ����. ��Ŀ�� �𵨷� ����] ---

    case WM_LBUTTONDOWN:
    {
        if (pData && pData->hStartMenu)
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            RECT rcButton;
            GetStartButtonRect(hWnd, &rcButton);

            if (PtInRect(&rcButton, pt)) // 1. ��ư ���� �ȿ��� Ŭ��
            {
                // ���� ���
                pData->bStartActive = !pData->bStartActive;

                if (pData->bStartActive)
                {
                    // Ȱ��: �޴� ǥ�� (�޴��� ������ SetFocus ��)
                    StartMenu_Show(pData->hStartMenu, hWnd);
                }
                else
                {
                    // ��Ȱ�� (��ư �ٽ� Ŭ��): �޴� �����
                    StartMenu_Hide(pData->hStartMenu);
                }

                InvalidateRect(hWnd, &rcButton, FALSE); // ��ư ��� �ٽ� �׸���
            }
            else // 2. ��ư ���� �� (�۾�ǥ���� ��)���� Ŭ��
            {
                if (pData->bStartActive)
                {
                    // �޴��� Ȱ��ȭ�Ǿ� �־��ٸ�, ��Ȱ��ȭ�ϰ� �޴� �����
                    // (StartMenu_Hide�� �ִϸ��̼Ǹ� ����, ������ �޴��� WM_KILLFOCUS�� �ް� WM_APP_MENU_CLOSED�� ����)
                    pData->bStartActive = FALSE;
                    StartMenu_Hide(pData->hStartMenu);
                    InvalidateRect(hWnd, &rcButton, FALSE); // ��ư ���� ��� ����
                }
            }
        }
        break;
    }

    // [����] �۾�ǥ���� ��Ŭ�� �ÿ��� �޴��� �����ִٸ� �ݱ�
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

    // [����] WM_KEYDOWN (ESC) �ڵ鷯�� SetCapture�� �Ⱦ��Ƿ� ���� (StartMenu�� ���� ó��)

    // [����] WM_CAPTURECHANGED �ڵ鷯�� WM_APP_MENU_CLOSED�� ��ü�Ǿ����Ƿ� ����

    // --- [���� ��] ---


    case WM_DESTROY:
    {
        APPBARDATA abd = {};
        abd.cbSize = sizeof(APPBARDATA);
        abd.hWnd = hWnd;
        SHAppBarMessage(ABM_REMOVE, &abd);

        // [�߰�] Taskbar�� ����� �� �ڽ�(������) ���� �޴� â�� �ı�
        if (pData && pData->hStartMenu)
        {
            DestroyWindow(pData->hStartMenu);
        }

        if (pData)
        {
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
 * @brief �۾�ǥ����(AppBar)�� ������ Ŭ������ �ý��ۿ� ����մϴ�. (���� ����)
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
    Gdiplus::Image* bg, Gdiplus::Image* start)
{
    g_hInstTaskbar = hInst;

    // --- [����: ���� ���� �ʱ�ȭ] ---
    TASKBAR_DATA* pData = new TASKBAR_DATA();
    pData->imgBackground = bg;
    pData->imgStart = start;
    pData->bStartHover = FALSE;
    pData->bMouseTracking = FALSE;
    pData->bStartActive = FALSE;
    pData->hStartMenu = NULL;        // [���� ����]
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

    // --- [�߰�: ���� �޴� â ����] ---
    pData->hStartMenu = StartMenu_Create(hInst, hWnd);
    if (!pData->hStartMenu)
    {
        DestroyWindow(hWnd);
        return FALSE;
    }
    // --- [�߰� ��] ---


    APPBARDATA abd = {};
    abd.cbSize = sizeof(APPBARDATA);
    abd.hWnd = hWnd;

    if (!SHAppBarMessage(ABM_NEW, &abd))
    {
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