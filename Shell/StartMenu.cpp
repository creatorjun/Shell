// creatorjun/shell/Shell-04869ccf080a38ea887a3d00139afecc32776daa/Shell/StartMenu.cpp

// StartMenu.cpp : ���� �޴� â ����, �ִϸ��̼� �� �̺�Ʈ ó���� �����մϴ�.
//

// windows.h �� GDI+�� min/max ��ũ�� �浹�� �����մϴ�.
#define NOMINMAX

#include "framework.h"
#include "StartMenu.h"      // �� ����� ��� (StartMenu Ŭ���� ����)
#include "GraphicsUtil.h"   // �ձ� �簢�� ���� �Լ��� ���� ����
#include "Constants.h"      // ��� ���� ���
#include <cmath>            // ceil()
#include <vector>
#include <string>
#include <memory>           // std::unique_ptr
#include <windowsx.h>       // GET_X_LPARAM, GET_Y_LPARAM
#include <shellapi.h>       // ExtractIconEx, ShellExecuteW

// --- [GDI+ ����] ---
#include <Objidl.h>          // IStream ���� ���� GDI+���� ���� ����
#include <gdiplus.h>         // GDI+ ���
using namespace Gdiplus;     // GDI+ ���ӽ����̽�
// --- [GDI+ ��] ---

#pragma comment(lib, "Shell32.lib") // ExtractIconEx, ShellExecuteW ��ũ

// [����] �͸� ���ӽ����̽��� ����Ͽ� ���� ������ ĸ��ȭ�մϴ�.
namespace
{

    // --- [���� ������ �� ���� ����] ---
    static const WCHAR START_MENU_CLASS[] = L"MyShell_StartMenuWindow";

    struct MenuItem
    {
        std::wstring text;
        std::wstring command;
        std::unique_ptr<Bitmap> iconBitmap;
        RECT         rcItem;
    };

    static HMODULE g_hImageres = NULL;
    static std::vector<MenuItem> g_menuItems;
    static std::unique_ptr<Font> g_fontItem;
    static std::unique_ptr<SolidBrush> g_brushText;
    static std::unique_ptr<SolidBrush> g_brushHover;
    static std::unique_ptr<SolidBrush> g_brushBackground;
    static int  g_hoverItem = -1;
    static BOOL g_mouseTracking = FALSE;

    struct STARTMENU_ANIM_STATE
    {
        BOOL  isAnimating;
        BOOL  isOpening;
        REAL  currentVisibleHeight;
        REAL  endHeight;
        REAL  stepHeightPerFrame;
    };
    static STARTMENU_ANIM_STATE g_AnimState = { 0 };

    // --- [���� ���� �Լ�] ---
    HICON LoadShellIcon(int iconIndex, int size)
    {
        HICON hIcon = NULL;
#pragma warning(push)
#pragma warning(disable: 6385)
        UINT iconsLoaded = PrivateExtractIconsW(
            L"imageres.dll", iconIndex, size, size, &hIcon, NULL, 1, 0
        );
#pragma warning(pop)
        if (iconsLoaded > 0 && hIcon) {
            return hIcon;
        }
        return NULL;
    }

    void CloseMenuAndNotify(HWND hWnd)
    {
        StartMenu::Hide(hWnd); // Ŭ���� ��� �Լ� ȣ��� ����
        HWND hOwner = GetWindow(hWnd, GW_OWNER);
        if (hOwner) {
            PostMessage(hOwner, WM_APP_MENU_CLOSED, 0, 0);
        }
    }

    // --- [���� ������ ���ν���] ---
    LRESULT CALLBACK StartMenu_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case WM_CREATE:
        {
            g_fontItem.reset(new Font(L"Segoe UI", 10.0f, FontStyleRegular, UnitPoint));
            g_brushText.reset(new SolidBrush(Color(255, 255, 255, 255)));
            g_brushHover.reset(new SolidBrush(Color(70, 255, 255, 255)));
            g_brushBackground.reset(new SolidBrush(Color(220, 40, 40, 45)));
            g_hImageres = LoadLibraryEx(L"imageres.dll", NULL, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_SEARCH_SYSTEM32);

            auto LoadAndConvertIcon = [](int iconIndex) -> std::unique_ptr<Bitmap> {
                HICON hIcon = LoadShellIcon(iconIndex, START_MENU_ICON_SIZE);
                if (!hIcon) return nullptr;
                std::unique_ptr<Bitmap> bmp(Bitmap::FromHICON(hIcon));
                DestroyIcon(hIcon);
                return bmp;
                };

            g_menuItems.push_back({ L"���� Ž����", L"explorer.exe", LoadAndConvertIcon(3), {0} });
            g_menuItems.push_back({ L"�͹̳�", L"cmd.exe", LoadAndConvertIcon(263), {0} });
            break;
        }
        case WM_TIMER:
        {
            if (wParam == ANIMATION_TIMER_ID && g_AnimState.isAnimating)
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
                    KillTimer(hWnd, ANIMATION_TIMER_ID);
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

            if (g_brushBackground) {
                memGfx.FillRectangle(g_brushBackground.get(), 0, 0, clientWidth, clientHeight);
            }

            int currentX = START_MENU_ITEM_PADDING;
            int currentY = START_MENU_ITEM_PADDING;
            for (size_t i = 0; i < g_menuItems.size(); ++i)
            {
                if ((currentX + START_MENU_ITEM_WIDTH) > (clientWidth - START_MENU_ITEM_PADDING)) {
                    currentX = START_MENU_ITEM_PADDING;
                    currentY += START_MENU_ITEM_HEIGHT + START_MENU_ITEM_PADDING;
                }
                SetRect(&g_menuItems[i].rcItem, currentX, currentY, currentX + START_MENU_ITEM_WIDTH, currentY + START_MENU_ITEM_HEIGHT);
                if (static_cast<int>(i) == g_hoverItem) {
                    GraphicsPath path;
                    RectF rcHoverF((REAL)g_menuItems[i].rcItem.left, (REAL)g_menuItems[i].rcItem.top,
                        (REAL)START_MENU_ITEM_WIDTH, (REAL)START_MENU_ITEM_HEIGHT);
                    CreateRoundedRectPath(&path, rcHoverF, 4.0f);
                    memGfx.FillPath(g_brushHover.get(), &path);
                }
                int iconPaddingX = (START_MENU_ITEM_WIDTH - START_MENU_ICON_SIZE) / 2;
                int iconTopPadding = 16;
                int iconX = g_menuItems[i].rcItem.left + iconPaddingX;
                int iconY = g_menuItems[i].rcItem.top + iconTopPadding;
                if (g_menuItems[i].iconBitmap) {
                    memGfx.DrawImage(g_menuItems[i].iconBitmap.get(),
                        iconX, iconY, START_MENU_ICON_SIZE, START_MENU_ICON_SIZE);
                }
                if (g_fontItem) {
                    float textY = (float)(iconY + START_MENU_ICON_SIZE + 8);
                    PointF textOrigin(
                        (REAL)(g_menuItems[i].rcItem.left + (START_MENU_ITEM_WIDTH / 2.0f)), textY);
                    StringFormat format;
                    format.SetAlignment(StringAlignmentCenter);
                    format.SetLineAlignment(StringAlignmentNear);
                    memGfx.DrawString(g_menuItems[i].text.c_str(), -1, g_fontItem.get(), textOrigin, &format, g_brushText.get());
                }
                currentX += START_MENU_ITEM_WIDTH + START_MENU_ITEM_PADDING;
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
                ShellExecuteW(hWnd, L"open", cmd, NULL, NULL, SW_SHOWNORMAL);
                CloseMenuAndNotify(hWnd);
            }
            break;
        }
        case WM_KEYDOWN:
        {
            if (wParam == VK_ESCAPE) {
                CloseMenuAndNotify(hWnd);
            }
        }
        break;
        case WM_KILLFOCUS:
        {
            CloseMenuAndNotify(hWnd);
        }
        break;
        case WM_CLOSE:
        {
            ShowWindow(hWnd, SW_HIDE); // â�� �ı����� �ʰ� ����⸸ �մϴ�.
            return 0;
        }
        case WM_DESTROY:
        {
            g_menuItems.clear();
            g_fontItem.reset();
            g_brushText.reset();
            g_brushHover.reset();
            g_brushBackground.reset();
            if (g_hImageres) {
                FreeLibrary(g_hImageres);
            }
            break;
        }
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        return 0;
    }

} // �͸� ���ӽ����̽� ��

// --- [StartMenu Ŭ������ Public ��� �Լ� ����] ---

ATOM StartMenu::Register(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = StartMenu_WndProc; // ���� WndProc ����
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

HWND StartMenu::Create(HINSTANCE hInstance, HWND hOwner)
{
    HWND hWnd = CreateWindowExW(
        WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        START_MENU_CLASS,
        L"Start Menu",
        WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        0, 0, START_MENU_WIDTH, START_MENU_HEIGHT,
        hOwner, nullptr, hInstance, nullptr
    );
    return hWnd;
}

void StartMenu::Show(HWND hStartMenu, HWND hTaskbar)
{
    if (IsWindowVisible(hStartMenu) && g_AnimState.isOpening) return;
    if (g_AnimState.isAnimating) KillTimer(hStartMenu, ANIMATION_TIMER_ID);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int finalTopY = screenHeight - (TASKBAR_HEIGHT + 12) - START_MENU_HEIGHT;
    int finalX = (screenWidth - START_MENU_WIDTH) / 2;

    SetWindowPos(hStartMenu, HWND_TOPMOST, finalX, finalTopY, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
    SetFocus(hStartMenu);

    g_AnimState.isAnimating = TRUE;
    g_AnimState.isOpening = TRUE;
    g_AnimState.currentVisibleHeight = 0.0f;
    g_AnimState.endHeight = (REAL)START_MENU_HEIGHT;

    float totalSteps = (float)(ANIMATION_DURATION_MS / ANIMATION_STEP_MS);
    g_AnimState.stepHeightPerFrame = ((REAL)START_MENU_HEIGHT / totalSteps);

    SetTimer(hStartMenu, ANIMATION_TIMER_ID, ANIMATION_STEP_MS, NULL);
}

void StartMenu::Hide(HWND hStartMenu)
{
    if (!IsWindowVisible(hStartMenu) && !g_AnimState.isOpening) return;
    if (g_AnimState.isAnimating) KillTimer(hStartMenu, ANIMATION_TIMER_ID);

    g_AnimState.isAnimating = TRUE;
    g_AnimState.isOpening = FALSE;
    g_AnimState.currentVisibleHeight = (REAL)START_MENU_HEIGHT;
    g_AnimState.endHeight = 0.0f;

    float totalSteps = (float)(ANIMATION_DURATION_MS / ANIMATION_STEP_MS);
    g_AnimState.stepHeightPerFrame = -((REAL)START_MENU_HEIGHT / totalSteps);

    SetTimer(hStartMenu, ANIMATION_TIMER_ID, ANIMATION_STEP_MS, NULL);
}