// creatorjun/shell/Shell-04869ccf080a38ea887a3d00139afecc32776daa/Shell/StartMenu.cpp

// StartMenu.cpp : 시작 메뉴 창 생성, 애니메이션 및 이벤트 처리를 구현합니다.
//

// windows.h 와 GDI+의 min/max 매크로 충돌을 방지합니다.
#define NOMINMAX

#include "framework.h"
#include "StartMenu.h"  // 이 모듈의 헤더
#include "GraphicsUtil.h" // [추가] 둥근 사각형 헬퍼 함수를 위해 포함
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


// --- [제거] ---
// CreateRoundedRectPath 함수는 GraphicsUtil.cpp로 이동했으므로 이 파일에 없어야 합니다.
// --- [제거 끝] ---


/**
 * @brief imageres.dll에서 아이콘을 로드하는 헬퍼 함수
 */
 // ExtractIconExW 대신 PrivateExtractIconsW를 사용하여 고해상도 아이콘을 로드합니다.
/**
 * @brief imageres.dll에서 아이콘을 로드하는 헬퍼 함수
 */
HICON LoadShellIcon(int iconIndex, int size)
{
    HICON hIcon = NULL;

    // C6385 경고는 이 상황에서 긍정 오류(False Positive)일 가능성이 높습니다.
    // 코드 분석기가 &hIcon이 단일 포인터임을 인지하고 잠재적 버퍼 오버런을 경고하지만,
    // 우리는 바로 아래 파라미터에서 1개의 아이콘만 로드하도록 명시하고 있습니다.
    // 따라서 이 경고를 일시적으로 비활성화합니다.
#pragma warning(push)
#pragma warning(disable: 6385)
    UINT iconsLoaded = PrivateExtractIconsW(
        L"imageres.dll",
        iconIndex,       // 가져올 아이콘 인덱스
        size,            // 원하는 너비
        size,            // 원하는 높이
        &hIcon,          // 아이콘 핸들을 저장할 포인터
        NULL,            // 아이콘 리소스 ID (필요 없음)
        1,               // 1개의 아이콘만 로드
        0                // 플래그
    );
#pragma warning(pop)

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
        g_brushText.reset(new SolidBrush(Color(255, 255, 255, 255)));
        g_brushHover.reset(new SolidBrush(Color(70, 255, 255, 255)));

        // 2. imageres.dll 로드
        g_hImageres = LoadLibraryEx(L"imageres.dll", NULL, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_SEARCH_SYSTEM32);

        // 3. 메뉴 아이템 데이터 및 아이콘 로드
        auto LoadAndConvertIcon = [](int iconIndex) -> std::unique_ptr<Bitmap> {
            HICON hIcon = LoadShellIcon(iconIndex, 32);
            if (!hIcon) return nullptr;

            std::unique_ptr<Bitmap> bmp(Bitmap::FromHICON(hIcon));
            DestroyIcon(hIcon);

            return bmp;
            }; // [수정] 람다 함수 정의는 반드시 세미콜론(;)으로 끝나야 합니다.

        g_menuItems.push_back({
            L"파일 탐색기",
            L"explorer.exe",
            LoadAndConvertIcon(3),
            {0}
            });

        g_menuItems.push_back({
            L"터미널",
            L"wt.exe",
            LoadAndConvertIcon(235),
            {0}
            });

        break;
    }

    case WM_TIMER:
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

        Bitmap memBitmap(clientWidth, clientHeight);
        Graphics memGfx(&memBitmap);

        memGfx.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
        memGfx.SetInterpolationMode(InterpolationModeHighQualityBicubic);
        memGfx.SetSmoothingMode(SmoothingModeAntiAlias);

        SolidBrush bgBrush(Color(220, 40, 40, 45));
        memGfx.FillRectangle(&bgBrush, 0, 0, clientWidth, clientHeight);

        int padding = 8;
        int itemWidth = 96;
        int itemHeight = 96;
        int iconSize = 32;
        int currentX = padding;
        int currentY = padding;

        for (size_t i = 0; i < g_menuItems.size(); ++i)
        {
            if ((currentX + itemWidth) > (clientWidth - padding))
            {
                currentX = padding;
                currentY += itemHeight + padding;
            }

            SetRect(&g_menuItems[i].rcItem, currentX, currentY, currentX + itemWidth, currentY + itemHeight);

            // [수정] size_t와 int의 비교 버그 수정
            if (static_cast<int>(i) == g_hoverItem)
            {
                GraphicsPath path;
                RectF rcHoverF((REAL)g_menuItems[i].rcItem.left, (REAL)g_menuItems[i].rcItem.top,
                    (REAL)itemWidth, (REAL)itemHeight);
                CreateRoundedRectPath(&path, rcHoverF, 4.0f);
                memGfx.FillPath(g_brushHover.get(), &path);
            }

            int iconPaddingX = (itemWidth - iconSize) / 2;
            int iconTopPadding = 16;
            int iconX = g_menuItems[i].rcItem.left + iconPaddingX;
            int iconY = g_menuItems[i].rcItem.top + iconTopPadding;

            if (g_menuItems[i].iconBitmap)
            {
                memGfx.DrawImage(
                    g_menuItems[i].iconBitmap.get(),
                    iconX, iconY, iconSize, iconSize
                );
            }

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

            currentX += itemWidth + padding;
        }

        Graphics screenGfx(hdc);
        RectF clipRect(0.0f, (REAL)clientHeight - (REAL)g_AnimState.currentVisibleHeight,
            (REAL)clientWidth, (REAL)g_AnimState.currentVisibleHeight);
        screenGfx.SetClip(clipRect);
        screenGfx.DrawImage(&memBitmap, 0, 0);

        EndPaint(hWnd, &ps);
    }
    break;

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
        for (size_t i = 0; i < g_menuItems.size(); ++i)
        {
            if (PtInRect(&g_menuItems[i].rcItem, pt))
            {
                currentHover = static_cast<int>(i);
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
        if (g_hoverItem != -1 && static_cast<size_t>(g_hoverItem) < g_menuItems.size())
        {
            LPCWSTR cmd = g_menuItems[g_hoverItem].command.c_str();
            HINSTANCE hRun = ShellExecuteW(hWnd, L"open", cmd, NULL, NULL, SW_SHOWNORMAL);

            if ((INT_PTR)hRun <= 32 && wcscmp(cmd, L"wt.exe") == 0)
            {
                ShellExecuteW(hWnd, L"open", L"cmd.exe", NULL, NULL, SW_SHOWNORMAL);
            }

            CloseMenuAndNotify(hWnd);
        }
        break;
    }

    case WM_KEYDOWN:
    {
        if (wParam == VK_ESCAPE)
        {
            CloseMenuAndNotify(hWnd);
        }
    }
    break;

    case WM_KILLFOCUS:
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

HWND StartMenu_Create(HINSTANCE hInstance, HWND hOwner)
{
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

void StartMenu_Show(HWND hStartMenu, HWND hTaskbar)
{
    if (IsWindowVisible(hStartMenu) && g_AnimState.isOpening) {
        return;
    }
    if (g_AnimState.isAnimating) {
        KillTimer(hStartMenu, ID_TIMER_ANIMATE);
    }

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int finalTopY = screenHeight - 60 - MENU_HEIGHT;
    int finalX = (screenWidth - MENU_WIDTH) / 2;

    SetWindowPos(hStartMenu, HWND_TOPMOST, finalX, finalTopY, 0, 0,
        SWP_NOSIZE | SWP_SHOWWINDOW);

    SetFocus(hStartMenu);

    g_AnimState.isAnimating = TRUE;
    g_AnimState.isOpening = TRUE;
    g_AnimState.currentVisibleHeight = 0.0f;
    g_AnimState.endHeight = (REAL)MENU_HEIGHT;

    float totalSteps = (float)(ANIMATION_DURATION_MS / ANIMATION_STEP_MS);
    g_AnimState.stepHeightPerFrame = ((REAL)MENU_HEIGHT / totalSteps);

    SetTimer(hStartMenu, ID_TIMER_ANIMATE, ANIMATION_STEP_MS, NULL);
}

void StartMenu_Hide(HWND hStartMenu)
{
    if (!IsWindowVisible(hStartMenu) && !g_AnimState.isOpening) {
        return;
    }
    if (g_AnimState.isAnimating) {
        KillTimer(hStartMenu, ID_TIMER_ANIMATE);
    }

    g_AnimState.isAnimating = TRUE;
    g_AnimState.isOpening = FALSE;
    g_AnimState.currentVisibleHeight = (REAL)MENU_HEIGHT;
    g_AnimState.endHeight = 0.0f;

    float totalSteps = (float)(ANIMATION_DURATION_MS / ANIMATION_STEP_MS);
    g_AnimState.stepHeightPerFrame = -((REAL)MENU_HEIGHT / totalSteps);

    SetTimer(hStartMenu, ID_TIMER_ANIMATE, ANIMATION_STEP_MS, NULL);
}