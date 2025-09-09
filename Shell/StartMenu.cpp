// creatorjun/shell/Shell-04869ccf080a38ea887a3d00139afecc32776daa/Shell/StartMenu.cpp

// StartMenu.cpp : ���� �޴� â ����, �ִϸ��̼� �� �̺�Ʈ ó���� �����մϴ�.
//

// windows.h �� GDI+�� min/max ��ũ�� �浹�� �����մϴ�.
#define NOMINMAX

#include "framework.h"
#include "StartMenu.h"  // �� ����� ���
#include "GraphicsUtil.h" // [�߰�] �ձ� �簢�� ���� �Լ��� ���� ����
#include <cmath>        // ceil()
#include <vector>
#include <string>
#include <memory>       // std::unique_ptr
#include <windowsx.h>   // GET_X_LPARAM, GET_Y_LPARAM
#include <shellapi.h>   // ExtractIconEx, ShellExecuteW

// --- [GDI+ ����] ---
#include <Objidl.h>          // IStream ���� ���� GDI+���� ���� ����
#include <gdiplus.h>         // GDI+ ���
using namespace Gdiplus;     // GDI+ ���ӽ����̽�
// --- [GDI+ ��] ---

#pragma comment(lib, "Shell32.lib") // ExtractIconEx, ShellExecuteW ��ũ
// PrivateExtractIconsW�� User32.lib (�⺻ ��ũ)�� ���ԵǾ� �ֽ��ϴ�.

// --- [����] ---
static const WCHAR START_MENU_CLASS[] = L"MyShell_StartMenuWindow";
const int MENU_WIDTH = 480;
const int MENU_HEIGHT = 560;

// Ÿ�̸� ����
const UINT_PTR ID_TIMER_ANIMATE = 1;      // Ÿ�̸� ID
const int ANIMATION_STEP_MS = 10;         // Ÿ�̸� ���� (10ms)
const int ANIMATION_DURATION_MS = 100;    // [���] 100ms�� ����

// --- [�޴� ������ ����ü] ---
struct MenuItem
{
    std::wstring text;       // ǥ�õ� �ؽ�Ʈ
    std::wstring command;    // ������ ��ɾ� (��: explorer.exe)
    std::unique_ptr<Bitmap> iconBitmap; // GDI+ ��Ʈ�� ��ü
    RECT         rcItem;     // �� �������� ��Ʈ�ڽ� RECT
};

// --- [��� ����(static) ����] ---

// ���ҽ� �ڵ�
static HMODULE g_hImageres = NULL;                  // imageres.dll ��� �ڵ�
static std::vector<MenuItem> g_menuItems;           // �޴� ������ ���
static std::unique_ptr<Font> g_fontItem;            // GDI+ �۲�
static std::unique_ptr<SolidBrush> g_brushText;     // �ؽ�Ʈ ����
static std::unique_ptr<SolidBrush> g_brushHover;    // ȣ�� ����

// ���� ����
static int  g_hoverItem = -1;       // ���� ���콺�� �ö� ������ �ε��� (-1 = ����)
static BOOL g_mouseTracking = FALSE; // WM_MOUSELEAVE ���� ����

// [Ŭ���� �ִϸ��̼� ����]
struct STARTMENU_ANIM_STATE
{
    BOOL  isAnimating;
    BOOL  isOpening;
    REAL  currentVisibleHeight;
    REAL  endHeight;
    REAL  stepHeightPerFrame;
};
static STARTMENU_ANIM_STATE g_AnimState = { 0 };
// --- [���� ���� ��] ---


// --- [����] ---
// CreateRoundedRectPath �Լ��� GraphicsUtil.cpp�� �̵������Ƿ� �� ���Ͽ� ����� �մϴ�.
// --- [���� ��] ---


/**
 * @brief imageres.dll���� �������� �ε��ϴ� ���� �Լ�
 */
 // ExtractIconExW ��� PrivateExtractIconsW�� ����Ͽ� ���ػ� �������� �ε��մϴ�.
/**
 * @brief imageres.dll���� �������� �ε��ϴ� ���� �Լ�
 */
HICON LoadShellIcon(int iconIndex, int size)
{
    HICON hIcon = NULL;

    // C6385 ���� �� ��Ȳ���� ���� ����(False Positive)�� ���ɼ��� �����ϴ�.
    // �ڵ� �м��Ⱑ &hIcon�� ���� ���������� �����ϰ� ������ ���� �������� ���������,
    // �츮�� �ٷ� �Ʒ� �Ķ���Ϳ��� 1���� �����ܸ� �ε��ϵ��� ����ϰ� �ֽ��ϴ�.
    // ���� �� ��� �Ͻ������� ��Ȱ��ȭ�մϴ�.
#pragma warning(push)
#pragma warning(disable: 6385)
    UINT iconsLoaded = PrivateExtractIconsW(
        L"imageres.dll",
        iconIndex,       // ������ ������ �ε���
        size,            // ���ϴ� �ʺ�
        size,            // ���ϴ� ����
        &hIcon,          // ������ �ڵ��� ������ ������
        NULL,            // ������ ���ҽ� ID (�ʿ� ����)
        1,               // 1���� �����ܸ� �ε�
        0                // �÷���
    );
#pragma warning(pop)

    if (iconsLoaded > 0 && hIcon)
    {
        return hIcon;
    }

    return NULL;
}


/**
 * @brief �޴� �������� �ݰ� �θ𿡰� �˸� (�ߺ� �ڵ� ���ſ� ����)
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
 * @brief ���� �޴��� �� ������ ���ν���(WndProc)
 */
LRESULT CALLBACK StartMenu_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        // 1. GDI+ ���ҽ� ����
        g_fontItem.reset(new Font(L"Segoe UI", 10.0f, FontStyleRegular, UnitPoint));
        g_brushText.reset(new SolidBrush(Color(255, 255, 255, 255)));
        g_brushHover.reset(new SolidBrush(Color(70, 255, 255, 255)));

        // 2. imageres.dll �ε�
        g_hImageres = LoadLibraryEx(L"imageres.dll", NULL, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_SEARCH_SYSTEM32);

        // 3. �޴� ������ ������ �� ������ �ε�
        auto LoadAndConvertIcon = [](int iconIndex) -> std::unique_ptr<Bitmap> {
            HICON hIcon = LoadShellIcon(iconIndex, 32);
            if (!hIcon) return nullptr;

            std::unique_ptr<Bitmap> bmp(Bitmap::FromHICON(hIcon));
            DestroyIcon(hIcon);

            return bmp;
            }; // [����] ���� �Լ� ���Ǵ� �ݵ�� �����ݷ�(;)���� ������ �մϴ�.

        g_menuItems.push_back({
            L"���� Ž����",
            L"explorer.exe",
            LoadAndConvertIcon(3),
            {0}
            });

        g_menuItems.push_back({
            L"�͹̳�",
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

            // [����] size_t�� int�� �� ���� ����
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