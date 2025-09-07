// Taskbar.cpp : �۾�ǥ����(AppBar) â ����, ��� �� �޽��� ó���� �����մϴ�.
//

// windows.h �� GDI+�� min/max ��ũ�� �浹�� �����մϴ�.
#define NOMINMAX

#include "framework.h"
#include "Taskbar.h"        // �� ����� ���
#include "StartButton.h"    // ���� ��ư �׸��� ��� ����
#include "Shell.h"          // ���ҽ� ID (IDM_ABOUT ��)
#include "Resource.h"       // ���ҽ� ID

#include <shellapi.h>       // APPBARDATA, SHAppBarMessage ���� ���� �ʼ�

// --- [GDI+ ����] ---
#include <Objidl.h>          // IStream ���� ���� GDI+���� ���� ����
#include <gdiplus.h>         // GDI+ ���
using namespace Gdiplus;     // GDI+ ���ӽ����̽�
// --- [GDI+ ��] ---


// --- [�ܺ� �Լ� ����] ---
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
// --- [���� ��] ---


static HINSTANCE g_hInstTaskbar;

struct TASKBAR_DATA
{
    Gdiplus::Image* imgBackground;
    Gdiplus::Image* imgStart;
};


/**
 * @brief �۾�ǥ����(AppBar)�� �� ������ ���ν���(WndProc)
 */
LRESULT CALLBACK Taskbar_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    TASKBAR_DATA* pData = (TASKBAR_DATA*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

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

    // --- [����ȭ ����: GDI+ ����� ���� ���� ���۸����� ����] ---
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps); // ȭ��(Screen) HDC

        // 1. Ŭ���̾�Ʈ ���� ũ�� ��������
        RECT rcClient;
        GetClientRect(hWnd, &rcClient);
        int clientWidth = rcClient.right - rcClient.left;
        int clientHeight = rcClient.bottom - rcClient.top;

        // 2. GDI+ ����� �޸� ��Ʈ�� ���� (��ǰ�� ARGB ǥ��)
        Bitmap memBitmap(clientWidth, clientHeight);

        // 3. GDI+ Graphics ��ü�� ȭ��(hdc)�� �ƴ� �޸� ��Ʈ��(memBitmap)���� ����
        Graphics* memGfx = Graphics::FromImage(&memBitmap);

        // 4. ��� �׸��⸦ �޸�(memGfx)�� ����

        // 4a. ��� �׸���
        if (pData && pData->imgBackground)
        {
            TextureBrush tBrush(pData->imgBackground);
            memGfx->FillRectangle(&tBrush, 0, 0, clientWidth, clientHeight);
        }
        else
        {
            // GDI+ �ε� ���� ��, (memGfx)�� GDI+�� �ܻ� ä���
            // (����: GDI�� (HBRUSH)(COLOR_WINDOW+1) ��� GDI+ Color ���)
            Color bgColor;
            bgColor.SetFromCOLORREF(GetSysColor(COLOR_WINDOW));
            SolidBrush fallbackBrush(bgColor);
            memGfx->FillRectangle(&fallbackBrush, 0, 0, clientWidth, clientHeight);
        }

        // 4b. ���� ��ư �׸��� (StartButton ����� ������ memory graphics ��ü�� ���)
        // (���� ��ǰ�� ���� ��� + ��ǰ�� ARGB ǥ�鿡 �׸��Ƿ� ǰ���� �ִ밡 ��)
        if (pData && pData->imgStart)
        {
            StartButton_Paint(memGfx, rcClient, pData->imgStart);
        }

        // (��� �׸��Ⱑ �Ϸ��)

        // 5. ȭ�� HDC�� Graphics ��ü�� ����
        Graphics screenGfx(hdc);

        // 6. �ϼ��� �׸�(memBitmap)�� ȭ��(screenGfx)���� �� ���� ����
        screenGfx.DrawImage(&memBitmap, 0, 0);

        // 7. �޸� �׷��Ƚ� ��ü ���� (memBitmap�� ���� ��ü�� �ڵ� �Ҹ�)
        delete memGfx;

        EndPaint(hWnd, &ps);
    }
    break;
    // --- [����ȭ ���� ��] ---

    case WM_DESTROY:
    {
        APPBARDATA abd = {};
        abd.cbSize = sizeof(APPBARDATA);
        abd.hWnd = hWnd;
        SHAppBarMessage(ABM_REMOVE, &abd);

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
 * @brief �۾�ǥ����(AppBar) �����츦 �����ϰ� ȭ�鿡 ǥ���մϴ�. (���� ����)
 */
BOOL Taskbar_Create(HINSTANCE hInst, int nCmdShow,
    const WCHAR* szTitle, const WCHAR* szWindowClass,
    Gdiplus::Image* bg, Gdiplus::Image* start)
{
    g_hInstTaskbar = hInst;

    TASKBAR_DATA* pData = new TASKBAR_DATA();
    pData->imgBackground = bg;
    pData->imgStart = start;

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