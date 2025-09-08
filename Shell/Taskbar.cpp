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
    Gdiplus::Image* imgBackground;
    Gdiplus::Image* imgStart;

    // 상태 관리 변수
    BOOL bStartHover;       // 마우스가 시작 버튼 위에 있는지 여부
    BOOL bMouseTracking;    // WM_MOUSELEAVE 이벤트를 추적 중인지 여부
    BOOL bStartActive;      // 시작 버튼이 활성화(클릭)되었는지 여부

    HWND hStartMenu;        // 생성된 시작 메뉴 윈도우 핸들
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
    // (StartMenu가 WM_KILLFOCUS, ESC, 아이템 클릭 등으로 닫힐 때 이 메시지를 보냄)
    if (message == WM_APP_MENU_CLOSED)
    {
        if (pData && pData->bStartActive) // 1. 활성 상태일 때만 처리
        {
            pData->bStartActive = FALSE; // 2. 활성 상태 해제

            // 3. (구 WM_CAPTURECHANGED 로직과 동일)
            //    메뉴가 닫힌 시점의 마우스가 버튼 위에 있는지 확인하여 호버 상태 갱신
            POINT ptCursor;
            GetCursorPos(&ptCursor);        // 화면(Screen) 좌표
            ScreenToClient(hWnd, &ptCursor); // 작업표시줄(Client) 좌표로 변환

            RECT rcButton;
            GetStartButtonRect(hWnd, &rcButton);

            BOOL bIsOverButton = PtInRect(&rcButton, ptCursor);
            if (bIsOverButton != pData->bStartHover)
            {
                pData->bStartHover = bIsOverButton; // 호버 상태 갱신
            }

            // 4. 최종 상태로 버튼 다시 그리기
            InvalidateRect(hWnd, &rcButton, FALSE);
        }
        return 0; // 메시지 처리 완료
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

    case WM_PAINT: // (수정 없음)
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT rcClient;
        GetClientRect(hWnd, &rcClient);
        int clientWidth = rcClient.right - rcClient.left;
        int clientHeight = rcClient.bottom - rcClient.top;

        Bitmap memBitmap(clientWidth, clientHeight);
        Graphics* memGfx = Graphics::FromImage(&memBitmap);

        // 배경 그리기
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

        // 시작 버튼 그리기
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

    // --- [마우스 추적 로직 (수정 없음)] ---
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
    // --- [마우스 추적 끝] ---


    // --- [수정: SetCapture 로직 완전 제거. 포커스 모델로 변경] ---

    case WM_LBUTTONDOWN:
    {
        if (pData && pData->hStartMenu)
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            RECT rcButton;
            GetStartButtonRect(hWnd, &rcButton);

            if (PtInRect(&rcButton, pt)) // 1. 버튼 영역 안에서 클릭
            {
                // 상태 토글
                pData->bStartActive = !pData->bStartActive;

                if (pData->bStartActive)
                {
                    // 활성: 메뉴 표시 (메뉴가 스스로 SetFocus 함)
                    StartMenu_Show(pData->hStartMenu, hWnd);
                }
                else
                {
                    // 비활성 (버튼 다시 클릭): 메뉴 숨기기
                    StartMenu_Hide(pData->hStartMenu);
                }

                InvalidateRect(hWnd, &rcButton, FALSE); // 버튼 즉시 다시 그리기
            }
            else // 2. 버튼 영역 밖 (작업표시줄 위)에서 클릭
            {
                if (pData->bStartActive)
                {
                    // 메뉴가 활성화되어 있었다면, 비활성화하고 메뉴 숨기기
                    // (StartMenu_Hide는 애니메이션만 실행, 닫히면 메뉴가 WM_KILLFOCUS를 받고 WM_APP_MENU_CLOSED를 보냄)
                    pData->bStartActive = FALSE;
                    StartMenu_Hide(pData->hStartMenu);
                    InvalidateRect(hWnd, &rcButton, FALSE); // 버튼 상태 즉시 복원
                }
            }
        }
        break;
    }

    // [수정] 작업표시줄 우클릭 시에도 메뉴가 열려있다면 닫기
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

    // [제거] WM_KEYDOWN (ESC) 핸들러는 SetCapture를 안쓰므로 제거 (StartMenu가 직접 처리)

    // [제거] WM_CAPTURECHANGED 핸들러는 WM_APP_MENU_CLOSED로 대체되었으므로 삭제

    // --- [수정 끝] ---


    case WM_DESTROY:
    {
        APPBARDATA abd = {};
        abd.cbSize = sizeof(APPBARDATA);
        abd.hWnd = hWnd;
        SHAppBarMessage(ABM_REMOVE, &abd);

        // [추가] Taskbar가 종료될 때 자식(소유된) 시작 메뉴 창도 파괴
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
 * @brief 작업표시줄(AppBar)의 윈도우 클래스를 시스템에 등록합니다. (수정 없음)
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
    Gdiplus::Image* bg, Gdiplus::Image* start)
{
    g_hInstTaskbar = hInst;

    // --- [수정: 상태 변수 초기화] ---
    TASKBAR_DATA* pData = new TASKBAR_DATA();
    pData->imgBackground = bg;
    pData->imgStart = start;
    pData->bStartHover = FALSE;
    pData->bMouseTracking = FALSE;
    pData->bStartActive = FALSE;
    pData->hStartMenu = NULL;        // [수정 없음]
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

    // --- [추가: 시작 메뉴 창 생성] ---
    pData->hStartMenu = StartMenu_Create(hInst, hWnd);
    if (!pData->hStartMenu)
    {
        DestroyWindow(hWnd);
        return FALSE;
    }
    // --- [추가 끝] ---


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