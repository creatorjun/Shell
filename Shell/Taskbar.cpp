// Taskbar.cpp : 작업표시줄(AppBar) 창 생성, 등록 및 메시지 처리를 구현합니다.
//

// windows.h 와 GDI+의 min/max 매크로 충돌을 방지합니다.
#define NOMINMAX

#include "framework.h"
#include "Taskbar.h"        // 이 모듈의 헤더
#include "StartButton.h"    // 시작 버튼 그리기 모듈 포함
#include "Shell.h"          // 리소스 ID (IDM_ABOUT 등)
#include "Resource.h"       // 리소스 ID

#include <shellapi.h>       // APPBARDATA, SHAppBarMessage 등을 위해 필수

// --- [GDI+ 설정] ---
#include <Objidl.h>          // IStream 등을 위해 GDI+보다 먼저 포함
#include <gdiplus.h>         // GDI+ 헤더
using namespace Gdiplus;     // GDI+ 네임스페이스
// --- [GDI+ 끝] ---


// --- [외부 함수 선언] ---
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
// --- [선언 끝] ---


static HINSTANCE g_hInstTaskbar;

struct TASKBAR_DATA
{
    Gdiplus::Image* imgBackground;
    Gdiplus::Image* imgStart;
};


/**
 * @brief 작업표시줄(AppBar)의 주 윈도우 프로시저(WndProc)
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

    // --- [최적화 수정: GDI+ 방식의 순수 더블 버퍼링으로 변경] ---
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps); // 화면(Screen) HDC

        // 1. 클라이언트 영역 크기 가져오기
        RECT rcClient;
        GetClientRect(hWnd, &rcClient);
        int clientWidth = rcClient.right - rcClient.left;
        int clientHeight = rcClient.bottom - rcClient.top;

        // 2. GDI+ 방식의 메모리 비트맵 생성 (고품질 ARGB 표면)
        Bitmap memBitmap(clientWidth, clientHeight);

        // 3. GDI+ Graphics 객체를 화면(hdc)이 아닌 메모리 비트맵(memBitmap)에서 생성
        Graphics* memGfx = Graphics::FromImage(&memBitmap);

        // 4. 모든 그리기를 메모리(memGfx)에 수행

        // 4a. 배경 그리기
        if (pData && pData->imgBackground)
        {
            TextureBrush tBrush(pData->imgBackground);
            memGfx->FillRectangle(&tBrush, 0, 0, clientWidth, clientHeight);
        }
        else
        {
            // GDI+ 로드 실패 시, (memGfx)에 GDI+로 단색 채우기
            // (참고: GDI의 (HBRUSH)(COLOR_WINDOW+1) 대신 GDI+ Color 사용)
            Color bgColor;
            bgColor.SetFromCOLORREF(GetSysColor(COLOR_WINDOW));
            SolidBrush fallbackBrush(bgColor);
            memGfx->FillRectangle(&fallbackBrush, 0, 0, clientWidth, clientHeight);
        }

        // 4b. 시작 버튼 그리기 (StartButton 모듈은 동일한 memory graphics 객체를 사용)
        // (이제 고품질 보간 모드 + 고품질 ARGB 표면에 그리므로 품질이 최대가 됨)
        if (pData && pData->imgStart)
        {
            StartButton_Paint(memGfx, rcClient, pData->imgStart);
        }

        // (모든 그리기가 완료됨)

        // 5. 화면 HDC용 Graphics 객체를 생성
        Graphics screenGfx(hdc);

        // 6. 완성된 그림(memBitmap)을 화면(screenGfx)으로 한 번에 복사
        screenGfx.DrawImage(&memBitmap, 0, 0);

        // 7. 메모리 그래픽스 객체 정리 (memBitmap은 스택 객체라 자동 소멸)
        delete memGfx;

        EndPaint(hWnd, &ps);
    }
    break;
    // --- [최적화 수정 끝] ---

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
 * @brief 작업표시줄(AppBar) 윈도우를 생성하고 화면에 표시합니다. (수정 없음)
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