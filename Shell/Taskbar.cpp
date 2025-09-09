// creatorjun/shell/Shell-04869ccf080a38ea887a3d00139afecc32776daa/Shell/Taskbar.cpp

// Taskbar.cpp : 작업표시줄(AppBar) 창 생성, 등록 및 메시지 처리를 구현합니다.
//

// windows.h 와 GDI+의 min/max 매크로 충돌을 방지합니다.
#define NOMINMAX

#include "framework.h"
#include "Taskbar.h"        // 이 모듈의 헤더
#include "StartButton.h"    // 시작 버튼 그리기 모듈 포함
#include "StartMenu.h"      // [추가] 시작 메뉴 모듈 헤더
#include "Shell.h"          // 리소스 ID (IDM_ABOUT 등)
#include "Resource.h"       // 리소스 ID

#include <shellapi.h>       // APPBARDATA, SHAppBarMessage 등을 위해 필수
#include <windowsx.h>       // GET_X_LPARAM, GET_Y_LPARAM 매크로 사용
#include <memory>           // [추가] std::unique_ptr 사용을 위해 포함

// --- [GDI+ 설정] ---
#include <Objidl.h>          // IStream 등을 위해 GDI+보다 먼저 포함
#include <gdiplus.h>         // GDI+ 헤더
using namespace Gdiplus;     // GDI+ 네임스페이스
// --- [GDI+ 끝] ---


// --- [외부 함수 선언] ---
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
// --- [선언 끝] ---


static HINSTANCE g_hInstTaskbar;

// --- [수정: TASKBAR_DATA 구조체] ---
struct TASKBAR_DATA
{
    std::unique_ptr<Gdiplus::Image> imgBackground;
    std::unique_ptr<Gdiplus::Image> imgStart;

    // 상태 관리 변수
    BOOL bStartHover;
    BOOL bMouseTracking;
    BOOL bStartActive;

    HWND hStartMenu;

    // [추가] 생성자를 정의하여 모든 멤버를 즉시 초기화합니다.
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
// --- [수정 끝] ---


// --- [시작 버튼 영역 계산 헬퍼] ---
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
// --- [함수 끝] ---


/**
 * @brief 작업표시줄(AppBar)의 주 윈도우 프로시저(WndProc)
 */
LRESULT CALLBACK Taskbar_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    TASKBAR_DATA* pData = (TASKBAR_DATA*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    // --- [새 핸들러: 포커스 모델용] ---
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
    // --- [새 핸들러 끝] ---


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
 * @brief 작업표시줄(AppBar)의 윈도우 클래스를 시스템에 등록합니다.
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
 * @brief 작업표시줄(AppBar) 윈도우를 생성하고 화면에 표시합니다.
 */
BOOL Taskbar_Create(HINSTANCE hInst, int nCmdShow,
    const WCHAR* szTitle, const WCHAR* szWindowClass,
    std::unique_ptr<Gdiplus::Image> bg, std::unique_ptr<Gdiplus::Image> start)
{
    g_hInstTaskbar = hInst;

    // --- [수정: RAII 및 생성자 활용] ---
    // 1. new TASKBAR_DATA()를 호출하면 새로 정의한 생성자가 모든 멤버를 초기화합니다.
    TASKBAR_DATA* pData = new TASKBAR_DATA();
    if (!pData) return FALSE;

    // 2. unique_ptr의 소유권을 pData의 멤버로 이동(move)
    pData->imgBackground = std::move(bg);
    pData->imgStart = std::move(start);

    // 3. [제거] 수동 초기화 코드가 더 이상 필요 없습니다.
    // pData->bStartHover = FALSE;
    // pData->bMouseTracking = FALSE;
    // pData->bStartActive = FALSE;
    // pData->hStartMenu = NULL;
    // --- [수정 끝] ---

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