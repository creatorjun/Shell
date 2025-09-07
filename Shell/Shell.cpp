// Shell.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "Shell.h"
#include <shellapi.h> // Shell API(AppBar)를 사용하기 위해 추가

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SHELL, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    MSG msg;

    // 기본 메시지 루프입니다: (단축키 처리 제거됨)
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SHELL));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr; // 메뉴 제거됨
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        [수정됨] AppBar의 높이를 45px로 변경합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

    // [수정] 작업 표시줄의 높이를 45로 변경
    int barHeight = 48;
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // AppBar 창 생성 (스타일은 동일)
    HWND hWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        szWindowClass,
        szTitle,
        WS_POPUP,
        0, 0,
        screenWidth, barHeight,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (!hWnd)
    {
        return FALSE;
    }

    // --- [수정] AppBar 등록 로직 (하단 기준, 45px 높이 적용) ---
    APPBARDATA abd = {};
    abd.cbSize = sizeof(APPBARDATA);
    abd.hWnd = hWnd;

    // 1. 새 AppBar로 시스템에 등록합니다.
    if (!SHAppBarMessage(ABM_NEW, &abd))
    {
        return FALSE;
    }

    // 2. AppBar 위치 및 크기 정의 (화면 하단, 45px)
    abd.uEdge = ABE_BOTTOM;
    abd.rc.left = 0;
    abd.rc.right = screenWidth;
    abd.rc.top = screenHeight - barHeight; // 화면 높이에서 45를 뺀 값
    abd.rc.bottom = screenHeight;

    // 3. 시스템에 해당 공간을 예약하도록 요청 (QUERYPOS)
    SHAppBarMessage(ABM_QUERYPOS, &abd);

    // 4. 시스템이 제안한 위치에 맞춰 우리 바의 높이(45px)를 '하단' 기준으로 다시 적용합니다.
    abd.rc.top = abd.rc.bottom - barHeight;

    // 5. 최종 위치를 시스템에 확정(SETPOS)하여 화면 공간을 예약합니다.
    SHAppBarMessage(ABM_SETPOS, &abd);

    // 6. 우리 창을 시스템이 지정한 최종 위치와 크기로 이동시킵니다.
    MoveWindow(hWnd, abd.rc.left, abd.rc.top, abd.rc.right - abd.rc.left, abd.rc.bottom - abd.rc.top, TRUE);
    // --- AppBar 등록 끝 ---

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // 메뉴 선택을 구문 분석합니다:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
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
        // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
    {
        // --- AppBar 등록 해제 ---
        APPBARDATA abd = {};
        abd.cbSize = sizeof(APPBARDATA);
        abd.hWnd = hWnd;
        SHAppBarMessage(ABM_REMOVE, &abd); // 시스템에서 AppBar 등록을 제거합니다.
        // --- ---

        PostQuitMessage(0);
    }
    break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
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