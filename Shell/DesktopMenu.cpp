// DesktopMenu.cpp : 바탕화면 우클릭 메뉴 창의 생성 및 메시지 처리를 구현합니다.
//

// windows.h 와 GDI+의 min/max 매크로 충돌을 방지합니다.
#define NOMINMAX

#include "framework.h"
#include "DesktopMenu.h"    // 이 모듈의 헤더
#include "Constants.h"      // 상수 정의 헤더
#include <windowsx.h>       // GET_X_LPARAM, GET_Y_LPARAM
#include <vector>           // std::vector
#include <string>           // std::wstring
#include <memory>           // std::unique_ptr
#include <shellapi.h>       // ShellExecuteW 사용을 위해 추가

// --- [GDI+ 설정] ---
#include <Objidl.h>          // IStream 등을 위해 GDI+보다 먼저 포함
#include <gdiplus.h>         // GDI+ 헤더
using namespace Gdiplus;     // GDI+ 네임스페이스
// --- [GDI+ 끝] ---

#pragma comment(lib, "Shell32.lib") // ShellExecuteW 링크

namespace
{
    // --- [내부 데이터 및 상태 변수] ---
    static const WCHAR DESKTOP_MENU_CLASS[] = L"MyShell_DesktopMenuWindow";
    constexpr int MENU_ITEM_HEIGHT = 32; // 메뉴 항목의 높이
    constexpr int MENU_ITEM_PADDING = 8;  // 메뉴 항목의 좌우 패딩

    struct MenuItem
    {
        std::wstring text;
        std::wstring command;
        RECT         rcItem;
    };

    static std::vector<MenuItem>       g_menuItems;
    static std::unique_ptr<Font>       g_fontItem;
    static std::unique_ptr<SolidBrush> g_brushText;
    static std::unique_ptr<SolidBrush> g_brushHover;
    static std::unique_ptr<SolidBrush> g_brushBackground;
    static int  g_hoverItem = -1;
    static BOOL g_mouseTracking = FALSE;


    // --- [내부 윈도우 프로시저] ---
    LRESULT CALLBACK DesktopMenu_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case WM_CREATE:
        {
            g_fontItem.reset(new Font(L"Segoe UI", 9.0f, FontStyleRegular, UnitPoint));
            g_brushText.reset(new SolidBrush(Color(255, 255, 255, 255)));
            g_brushHover.reset(new SolidBrush(Color(70, 255, 255, 255)));
            g_brushBackground.reset(new SolidBrush(Color(220, 40, 40, 45)));

            g_menuItems.push_back({ L"Display settings", L"desk.cpl", {0} });
            g_menuItems.push_back({ L"Terminal", L"cmd.exe", {0} });
            g_menuItems.push_back({ L"Shutdown", L"shutdown /s /t 0", {0} });
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
            memGfx.SetSmoothingMode(SmoothingModeAntiAlias);

            memGfx.FillRectangle(g_brushBackground.get(), 0, 0, clientWidth, clientHeight);

            for (size_t i = 0; i < g_menuItems.size(); ++i)
            {
                // [수정] C4267 경고 해결을 위해 static_cast 사용
                int itemTop = static_cast<int>(i) * MENU_ITEM_HEIGHT;
                int itemBottom = itemTop + MENU_ITEM_HEIGHT;
                SetRect(&g_menuItems[i].rcItem, 0, itemTop, clientWidth, itemBottom);

                if (static_cast<int>(i) == g_hoverItem) {
                    // [수정] C2668 오류 해결을 위해 Gdiplus::Rect 객체 사용
                    Gdiplus::Rect hoverRect(g_menuItems[i].rcItem.left, g_menuItems[i].rcItem.top,
                        g_menuItems[i].rcItem.right - g_menuItems[i].rcItem.left,
                        g_menuItems[i].rcItem.bottom - g_menuItems[i].rcItem.top);
                    memGfx.FillRectangle(g_brushHover.get(), hoverRect);
                }

                StringFormat strFormat;
                strFormat.SetAlignment(StringAlignmentNear);
                strFormat.SetLineAlignment(StringAlignmentCenter);
                PointF textOrigin((REAL)MENU_ITEM_PADDING, (REAL)g_menuItems[i].rcItem.top + (MENU_ITEM_HEIGHT / 2.0f));
                memGfx.DrawString(g_menuItems[i].text.c_str(), -1, g_fontItem.get(), textOrigin, &strFormat, g_brushText.get());
            }

            Graphics screenGfx(hdc);
            screenGfx.DrawImage(&memBitmap, 0, 0);

            EndPaint(hWnd, &ps);
            break;
        }
        case WM_MOUSEMOVE:
        {
            if (!g_mouseTracking) {
                TRACKMOUSEEVENT tme = { sizeof(tme) };
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = hWnd;
                if (TrackMouseEvent(&tme)) {
                    g_mouseTracking = TRUE;
                }
            }
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            int currentHover = -1;
            for (size_t i = 0; i < g_menuItems.size(); ++i) {
                if (PtInRect(&g_menuItems[i].rcItem, pt)) {
                    currentHover = static_cast<int>(i);
                    break;
                }
            }
            if (currentHover != g_hoverItem) {
                g_hoverItem = currentHover;
                InvalidateRect(hWnd, NULL, FALSE);
            }
            break;
        }
        case WM_MOUSELEAVE:
        {
            g_mouseTracking = FALSE;
            if (g_hoverItem != -1) {
                g_hoverItem = -1;
                InvalidateRect(hWnd, NULL, FALSE);
            }
            break;
        }
        case WM_LBUTTONDOWN:
        {
            if (g_hoverItem != -1 && static_cast<size_t>(g_hoverItem) < g_menuItems.size()) {
                LPCWSTR cmd = g_menuItems[g_hoverItem].command.c_str();

                std::wstring commandStr(cmd);
                size_t firstSpace = commandStr.find(L' ');
                std::wstring executable;
                std::wstring args;

                if (firstSpace != std::wstring::npos) {
                    executable = commandStr.substr(0, firstSpace);
                    args = commandStr.substr(firstSpace + 1);
                }
                else {
                    executable = commandStr;
                }

                ShellExecuteW(hWnd, L"open", executable.c_str(), args.empty() ? NULL : args.c_str(), NULL, SW_SHOWNORMAL);
                DesktopMenu::Hide(hWnd);
            }
            break;
        }
        case WM_KILLFOCUS:
        {
            DesktopMenu::Hide(hWnd);
            break;
        }
        case WM_DESTROY:
        {
            g_menuItems.clear();
            g_fontItem.reset();
            g_brushText.reset();
            g_brushHover.reset();
            g_brushBackground.reset();
            break;
        }
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        return 0;
    }
}


ATOM DesktopMenu::Register(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = DesktopMenu_WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = NULL;
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = DESKTOP_MENU_CLASS;
    wcex.hIconSm = NULL;
    return RegisterClassExW(&wcex);
}

HWND DesktopMenu::Create(HINSTANCE hInstance, HWND hOwner)
{
    int menuWidth = 200;
    // [수정] C4267 경고 해결을 위해 static_cast 사용
    int menuHeight = g_menuItems.empty() ? 100 : static_cast<int>(g_menuItems.size()) * MENU_ITEM_HEIGHT;

    HWND hWnd = CreateWindowExW(
        WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        DESKTOP_MENU_CLASS,
        L"Desktop Menu",
        WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        0, 0, menuWidth, menuHeight,
        hOwner, nullptr, hInstance, nullptr
    );

    if (hWnd)
    {
        // [수정] C4267 경고 해결을 위해 static_cast 사용
        menuHeight = static_cast<int>(g_menuItems.size()) * MENU_ITEM_HEIGHT;
        SetWindowPos(hWnd, NULL, 0, 0, menuWidth, menuHeight, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    }

    return hWnd;
}

void DesktopMenu::Show(HWND hMenu, int x, int y)
{
    if (!IsWindow(hMenu)) return;

    SetWindowPos(hMenu, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
    SetForegroundWindow(hMenu);
    SetFocus(hMenu);
}

void DesktopMenu::Hide(HWND hMenu)
{
    if (!IsWindow(hMenu)) return;

    ShowWindow(hMenu, SW_HIDE);
}