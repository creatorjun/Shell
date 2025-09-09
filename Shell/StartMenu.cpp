// creatorjun/shell/Shell-04869ccf080a38ea887a3d00139afecc32776daa/Shell/StartMenu.cpp

// StartMenu.cpp : 시작 메뉴 창 생성, 애니메이션 및 이벤트 처리를 구현합니다.
//

// windows.h 와 GDI+의 min/max 매크로 충돌을 방지합니다.
#define NOMINMAX

#include "framework.h"
#include "StartMenu.h"  // 이 모듈의 헤더
#include <cmath>        // ceil()
#include <vector>
#include <string>
#include <memory>       // std::unique_ptr
#include <windowsx.h>   // GET_X_LPARAM, GET_Y_LPARAM
#include <shellapi.h>   // ExtractIconEx, ShellExecuteW

// --- [GDI+ 설정] ---
#include <Objidl.h>          // IStream 등을 위해 GDI+보다 먼저 포함
#include <gdiplus.h>         // GDI+ 헤더
using namespace Gdiplus;     // GDI+ 네임스페이스
// --- [GDI+ 끝] ---

#pragma comment(lib, "Shell32.lib") // ExtractIconEx, ShellExecuteW 링크
// PrivateExtractIconsW는 User32.lib (기본 링크)에 포함되어 있습니다.

// --- [설정] ---
static const WCHAR START_MENU_CLASS[] = L"MyShell_StartMenuWindow";
const int MENU_WIDTH = 480;
const int MENU_HEIGHT = 560;

// 타이머 설정
const UINT_PTR ID_TIMER_ANIMATE = 1;      // 타이머 ID
const int ANIMATION_STEP_MS = 10;         // 타이머 간격 (10ms)
const int ANIMATION_DURATION_MS = 100;    // [기억] 100ms로 설정

// --- [메뉴 아이템 구조체] ---
struct MenuItem
{
    std::wstring text;       // 표시될 텍스트
    std::wstring command;    // 실행할 명령어 (예: explorer.exe)
    std::unique_ptr<Bitmap> iconBitmap; // GDI+ 비트맵 객체
    RECT         rcItem;     // 이 아이템의 히트박스 RECT
};

// --- [모듈 전역(static) 변수] ---

// 리소스 핸들
static HMODULE g_hImageres = NULL;                  // imageres.dll 모듈 핸들
static std::vector<MenuItem> g_menuItems;           // 메뉴 아이템 목록
static std::unique_ptr<Font> g_fontItem;            // GDI+ 글꼴
static std::unique_ptr<SolidBrush> g_brushText;     // 텍스트 색상
static std::unique_ptr<SolidBrush> g_brushHover;    // 호버 배경색

// 상태 변수
static int  g_hoverItem = -1;       // 현재 마우스가 올라간 아이템 인덱스 (-1 = 없음)
static BOOL g_mouseTracking = FALSE; // WM_MOUSELEAVE 추적 여부

// [클리핑 애니메이션 상태]
struct STARTMENU_ANIM_STATE
{
    BOOL  isAnimating;
    BOOL  isOpening;
    REAL  currentVisibleHeight;
    REAL  endHeight;
    REAL  stepHeightPerFrame;
};
static STARTMENU_ANIM_STATE g_AnimState = { 0 };
// --- [전역 변수 끝] ---


/**
 * @brief (StartButton.cpp에서 복사) 둥근 사각형 경로 헬퍼
 */
void CreateRoundedRectPath(GraphicsPath* path, RectF rect, REAL cornerRadius)
{
    if (!path) return;
    REAL dia = cornerRadius * 2.0f;
    if (dia <= 0.0f) {
        path->AddRectangle(rect);
        return;
    }
    path->AddArc(rect.X, rect.Y, dia, dia, 180, 90);
    path->AddArc(rect.GetRight() - dia, rect.Y, dia, dia, 270, 90);
    path->AddArc(rect.GetRight() - dia, rect.GetBottom() - dia, dia, dia, 0, 90);
    path->AddArc(rect.X, rect.GetBottom() - dia, dia, dia, 90, 90);
    path->CloseFigure();
}

/**
 * @brief imageres.dll에서 아이콘을 로드하는 헬퍼 함수
 */
 // ExtractIconExW 대신 PrivateExtractIconsW를 사용하여 고해상도 아이콘을 로드합니다.
HICON LoadShellIcon(int iconIndex, int size)
{
    HICON hIcon = NULL;

    // 256x256 크기를 요청하여 사용 가능한 가장 큰 아이콘 리소스를 가져옵니다.
    UINT iconsLoaded = PrivateExtractIconsW(
        L"imageres.dll",
        iconIndex,       // 가져올 아이콘 인덱스
        size,            // 원하는 너비 (예: 256)
        size,            // 원하는 높이 (예: 256)
        &hIcon,          // 아이콘 핸들을 저장할 포인터
        NULL,            // 아이콘 리소스 ID (필요 없음)
        1,               // 1개의 아이콘만 로드
        0                // 플래그 (LR_DEFAULTCOLOR 등)
    );

    if (iconsLoaded > 0 && hIcon)
    {
        return hIcon;
    }

    return NULL;
}


/**
 * @brief 메뉴 아이템을 닫고 부모에게 알림 (중복 코드 제거용 헬퍼)
 */
void CloseMenuAndNotify(HWND hWnd)
{
    StartMenu_Hide(hWnd);
    HWND hOwner = GetWindow(hWnd, GW_OWNER);
    if (hOwner)
    {
        PostMessage(hOwner, WM_APP_MENU_CLOSED, 0, 0);
    }
}


/**
 * @brief 시작 메뉴의 주 윈도우 프로시저(WndProc)
 */
LRESULT CALLBACK StartMenu_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        // 1. GDI+ 리소스 생성
        g_fontItem.reset(new Font(L"Segoe UI", 10.0f, FontStyleRegular, UnitPoint));
        g_brushText.reset(new SolidBrush(Color(255, 255, 255, 255))); // 흰색 텍스트
        g_brushHover.reset(new SolidBrush(Color(70, 255, 255, 255))); // 약 27% 흰색 호버

        // 2. imageres.dll 로드
        g_hImageres = LoadLibraryEx(L"imageres.dll", NULL, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_SEARCH_SYSTEM32);

        // 3. 메뉴 아이템 데이터 및 아이콘 로드

        // 헬퍼 람다: HICON을 로드하고 GDI+ Bitmap으로 변환한 뒤 HICON을 파괴합니다.
        auto LoadAndConvertIcon = [](int iconIndex) -> std::unique_ptr<Bitmap> {

            // --- [선명도 수정] ---
            // 256px 대신 32px 아이콘을 직접 요청합니다.
            // 이렇게 하면 리사이징(보간)이 발생하지 않아 가장 선명합니다.
            HICON hIcon = LoadShellIcon(iconIndex, 32); // <--- [수정됨] 256에서 32로 변경

            if (!hIcon) return nullptr;

            std::unique_ptr<Bitmap> bmp(Bitmap::FromHICON(hIcon));
            DestroyIcon(hIcon);

            return bmp; // 32x32 비트맵의 unique_ptr 반환
            };

        g_menuItems.push_back({
            L"파일 탐색기",
            L"explorer.exe",
            LoadAndConvertIcon(3),
            {0}
            });

        g_menuItems.push_back({
            L"터미널",
            L"wt.exe",
            LoadAndConvertIcon(255),
            {0}
            });

        break;
    }

    case WM_TIMER: // (클리핑 애니메이션 로직 - 수정 없음)
    {
        if (wParam == ID_TIMER_ANIMATE && g_AnimState.isAnimating)
        {
            g_AnimState.currentVisibleHeight += g_AnimState.stepHeightPerFrame;

            BOOL bFinished = FALSE;
            if (g_AnimState.isOpening) {
                if (g_AnimState.currentVisibleHeight >= g_AnimState.endHeight) bFinished = TRUE;
            }
            else {
                if (g_AnimState.currentVisibleHeight <= g_AnimState.endHeight) bFinished = TRUE;
            }

            if (bFinished) {
                KillTimer(hWnd, ID_TIMER_ANIMATE);
                g_AnimState.isAnimating = FALSE;
                g_AnimState.currentVisibleHeight = g_AnimState.endHeight;
                if (!g_AnimState.isOpening) {
                    ShowWindow(hWnd, SW_HIDE);
                }
            }
            InvalidateRect(hWnd, NULL, FALSE);
        }
        break;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT rcClient;
        GetClientRect(hWnd, &rcClient);
        int clientWidth = rcClient.right - rcClient.left;
        int clientHeight = rcClient.bottom - rcClient.top;

        // 1. 더블 버퍼링용 인-메모리 비트맵 및 Graphics 객체 생성
        Bitmap memBitmap(clientWidth, clientHeight);
        Graphics memGfx(&memBitmap);

        memGfx.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

        // --- [아이콘 선명도 수정] ---
        // (이제 32x32 아이콘을 직접 로드하므로 보간 모드 설정은 
        //  사실상 드로잉 품질에 영향을 주지 않지만, 최고 품질로 유지합니다.)
        memGfx.SetInterpolationMode(InterpolationModeHighQualityBicubic);
        // --- [수정 끝] ---

        memGfx.SetSmoothingMode(SmoothingModeAntiAlias);

        // 2. 메모리에 전체 배경 그리기
        SolidBrush bgBrush(Color(220, 40, 40, 45));
        memGfx.FillRectangle(&bgBrush, 0, 0, clientWidth, clientHeight);

        // --- [바둑판식 레이아웃] ---

        // 3. 그리드 레이아웃 설정
        int padding = 8;        // 아이템 간 간격
        int itemWidth = 96;     // 각 타일의 너비
        int itemHeight = 96;    // 각 타일의 높이
        int iconSize = 32;      // 아이콘 목표 크기

        int currentX = padding; // 현재 그리기 시작할 X 위치
        int currentY = padding; // 현재 그리기 시작할 Y 위치

        for (int i = 0; i < g_menuItems.size(); ++i)
        {
            // 3a. 줄바꿈 처리
            if ((currentX + itemWidth) > (clientWidth - padding))
            {
                currentX = padding;
                currentY += itemHeight + padding;
            }

            // 3b. 아이템 히트박스(RECT) 계산 및 저장
            SetRect(&g_menuItems[i].rcItem, currentX, currentY, currentX + itemWidth, currentY + itemHeight);

            // 3c. 호버 효과 그리기
            if (i == g_hoverItem)
            {
                GraphicsPath path;
                RectF rcHoverF((REAL)g_menuItems[i].rcItem.left, (REAL)g_menuItems[i].rcItem.top,
                    (REAL)itemWidth, (REAL)itemHeight);
                CreateRoundedRectPath(&path, rcHoverF, 4.0f);
                memGfx.FillPath(g_brushHover.get(), &path);
            }

            // 3d. 아이콘 그리기 (타일 상단 중앙 정렬)
            int iconPaddingX = (itemWidth - iconSize) / 2;    // (96 - 32) / 2 = 32
            int iconTopPadding = 16;                          // 아이콘 상단 여백
            int iconX = g_menuItems[i].rcItem.left + iconPaddingX;
            int iconY = g_menuItems[i].rcItem.top + iconTopPadding;

            if (g_menuItems[i].iconBitmap)
            {
                // 이제 32x32 비트맵을 32x32 크기로 '그대로' 그립니다. (1:1 매칭)
                memGfx.DrawImage(
                    g_menuItems[i].iconBitmap.get(),
                    iconX,
                    iconY,
                    iconSize,  // 32
                    iconSize   // 32
                );
            }

            // 3e. 텍스트 그리기 (아이콘 아래, 타일 중앙 정렬)
            if (g_fontItem)
            {
                float textY = (float)(iconY + iconSize + 8);
                PointF textOrigin(
                    (REAL)(g_menuItems[i].rcItem.left + (itemWidth / 2.0f)),
                    textY
                );

                StringFormat format;
                format.SetAlignment(StringAlignmentCenter);
                format.SetLineAlignment(StringAlignmentNear);

                memGfx.DrawString(g_menuItems[i].text.c_str(), -1, g_fontItem.get(), textOrigin, &format, g_brushText.get());
            }

            // 3f. 다음 아이템을 위해 X 위치 이동
            currentX += itemWidth + padding;
        }
        // --- [수정 끝] ---


        // 4. 화면 HDC용 Graphics 객체 생성
        Graphics screenGfx(hdc);

        // 5. 클리핑 영역(보이는 영역) 설정
        RectF clipRect(0.0f, (REAL)clientHeight - (REAL)g_AnimState.currentVisibleHeight,
            (REAL)clientWidth, (REAL)g_AnimState.currentVisibleHeight);
        screenGfx.SetClip(clipRect);

        // 6. 완성된 메모리 비트맵을 화면으로 전송 (클립 영역만 그려짐)
        screenGfx.DrawImage(&memBitmap, 0, 0);

        EndPaint(hWnd, &ps);
    }
    break;

    // --- [포커스 모델용 입력 처리] ---

    case WM_MOUSEMOVE:
    {
        if (!g_mouseTracking)
        {
            TRACKMOUSEEVENT tme = { sizeof(tme) };
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = hWnd;
            if (TrackMouseEvent(&tme))
            {
                g_mouseTracking = TRUE;
            }
        }

        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        int currentHover = -1;
        for (int i = 0; i < g_menuItems.size(); ++i)
        {
            if (PtInRect(&g_menuItems[i].rcItem, pt))
            {
                currentHover = i;
                break;
            }
        }

        if (currentHover != g_hoverItem)
        {
            g_hoverItem = currentHover;
            InvalidateRect(hWnd, NULL, FALSE);
        }
        break;
    }

    case WM_MOUSELEAVE:
    {
        g_mouseTracking = FALSE;
        if (g_hoverItem != -1)
        {
            g_hoverItem = -1;
            InvalidateRect(hWnd, NULL, FALSE);
        }
        break;
    }

    case WM_LBUTTONDOWN:
    {
        if (g_hoverItem != -1) // 유효한 아이템 위에서 클릭
        {
            LPCWSTR cmd = g_menuItems[g_hoverItem].command.c_str();
            HINSTANCE hRun = ShellExecuteW(hWnd, L"open", cmd, NULL, NULL, SW_SHOWNORMAL);

            if ((INT_PTR)hRun <= 32 && wcscmp(cmd, L"wt.exe") == 0) // wt.exe 실행 실패 시
            {
                ShellExecuteW(hWnd, L"open", L"cmd.exe", NULL, NULL, SW_SHOWNORMAL); // cmd.exe로 대체
            }

            CloseMenuAndNotify(hWnd);
        }
        break;
    }

    case WM_KEYDOWN: // ESC
    {
        if (wParam == VK_ESCAPE)
        {
            CloseMenuAndNotify(hWnd);
        }
    }
    break;

    case WM_KILLFOCUS: // 포커스 잃음 (Click-away)
    {
        CloseMenuAndNotify(hWnd);
    }
    break;

    case WM_DESTROY:
    {
        g_menuItems.clear();

        if (g_hImageres)
        {
            FreeLibrary(g_hImageres);
        }
        break;
    }

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


/**
 * @brief 시작 메뉴 윈도우 클래스를 등록합니다.
 */
ATOM StartMenu_Register(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = StartMenu_WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = NULL;
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = START_MENU_CLASS;
    wcex.hIconSm = NULL;

    return RegisterClassExW(&wcex);
}

/**
 * @brief 시작 메뉴 윈도우를 (숨겨진 상태로) 생성합니다.
 */
HWND StartMenu_Create(HINSTANCE hInstance, HWND hOwner)
{
    // [수정] WS_EX_NOACTIVATE 플래그 제거 (포커스와 클릭을 받아야 함)
    HWND hWnd = CreateWindowExW(
        WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        START_MENU_CLASS,
        L"Start Menu",
        WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        0, 0, MENU_WIDTH, MENU_HEIGHT,
        hOwner,
        nullptr,
        hInstance,
        nullptr
    );

    return hWnd;
}


// --- [포커스 모델용 애니메이션 로직] ---

/**
 * @brief [수정] 시작 메뉴를 클리핑 애니메이션과 함께 표시하고 "포커스"를 설정합니다.
 */
void StartMenu_Show(HWND hStartMenu, HWND hTaskbar)
{
    if (IsWindowVisible(hStartMenu) && g_AnimState.isOpening) {
        return;
    }
    if (g_AnimState.isAnimating) {
        KillTimer(hStartMenu, ID_TIMER_ANIMATE);
    }

    // 1. 창을 최종 위치로 즉시 이동시키고 표시
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int finalTopY = screenHeight - 60 - MENU_HEIGHT;
    int finalX = (screenWidth - MENU_WIDTH) / 2;

    SetWindowPos(hStartMenu, HWND_TOPMOST, finalX, finalTopY, 0, 0,
        SWP_NOSIZE | SWP_SHOWWINDOW);

    // 2. [핵심] 창에 즉시 포커스를 설정
    SetFocus(hStartMenu);

    // 3. 애니메이션 상태 변수 설정 (Height: 0 -> MAX)
    g_AnimState.isAnimating = TRUE;
    g_AnimState.isOpening = TRUE;
    g_AnimState.currentVisibleHeight = 0.0f;
    g_AnimState.endHeight = (REAL)MENU_HEIGHT;

    float totalSteps = (float)(ANIMATION_DURATION_MS / ANIMATION_STEP_MS);
    g_AnimState.stepHeightPerFrame = ((REAL)MENU_HEIGHT / totalSteps);

    // 4. 애니메이션 타이머 시작
    SetTimer(hStartMenu, ID_TIMER_ANIMATE, ANIMATION_STEP_MS, NULL);
}

/**
 * @brief [수정] 시작 메뉴를 클리핑 애니메이션과 함께 숨깁니다.
 */
void StartMenu_Hide(HWND hStartMenu)
{
    if (!IsWindowVisible(hStartMenu) && !g_AnimState.isOpening) {
        return;
    }
    if (g_AnimState.isAnimating) {
        KillTimer(hStartMenu, ID_TIMER_ANIMATE);
    }

    // 1. 애니메이션 상태 변수 설정 (Height: MAX -> 0)
    g_AnimState.isAnimating = TRUE;
    g_AnimState.isOpening = FALSE;
    g_AnimState.currentVisibleHeight = (REAL)MENU_HEIGHT;
    g_AnimState.endHeight = 0.0f;

    float totalSteps = (float)(ANIMATION_DURATION_MS / ANIMATION_STEP_MS);
    g_AnimState.stepHeightPerFrame = -((REAL)MENU_HEIGHT / totalSteps);

    // 2. 애니메이션 타이머 시작
    SetTimer(hStartMenu, ID_TIMER_ANIMATE, ANIMATION_STEP_MS, NULL);
}
// --- [수정 끝] ---