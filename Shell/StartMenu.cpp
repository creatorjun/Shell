// creatorjun/shell/Shell-04869ccf080a38ea887a3d00139afecc32776daa/Shell/StartMenu.cpp

// StartMenu.cpp : ���� �޴� â ����, �ִϸ��̼� �� �̺�Ʈ ó���� �����մϴ�.
//

// windows.h �� GDI+�� min/max ��ũ�� �浹�� �����մϴ�.
#define NOMINMAX

#include "framework.h"
#include "StartMenu.h"  // �� ����� ���
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
    HICON        hIcon;      // GDI+�� �׸� 32x32 ������ �ڵ�
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


/**
 * @brief (StartButton.cpp���� ����) �ձ� �簢�� ��� ����
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
 * @brief imageres.dll���� �������� �ε��ϴ� ���� �Լ�
 */
HICON LoadShellIcon(int iconIndex, int size)
{
    if (!g_hImageres) return NULL;
    HICON hIcon = NULL;
    ExtractIconExW(L"imageres.dll", iconIndex, NULL, &hIcon, 1);
    return hIcon;
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
        g_brushText.reset(new SolidBrush(Color(255, 255, 255, 255))); // ��� �ؽ�Ʈ
        g_brushHover.reset(new SolidBrush(Color(70, 255, 255, 255))); // �� 27% ��� ȣ��
        
        // 2. imageres.dll �ε�
        g_hImageres = LoadLibraryEx(L"imageres.dll", NULL, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_SEARCH_SYSTEM32);

        // 3. �޴� ������ ������ �� ������ �ε�
        if (g_hImageres)
        {
            g_menuItems.push_back({
                L"���� Ž����", 
                L"explorer.exe", 
                LoadShellIcon(128, 32), // 128 = This PC/Computer
                {0} 
            });

            g_menuItems.push_back({ 
                L"�͹̳�", 
                L"wt.exe", // (Windows Terminal. ���� �� cmd.exe�� ��ü ����)
                LoadShellIcon(6, 32), // 6 = Command Prompt
                {0} 
            });
        }
        break;
    }

    case WM_TIMER: // (Ŭ���� �ִϸ��̼� ���� - ���� ����)
    {
        if (wParam == ID_TIMER_ANIMATE && g_AnimState.isAnimating)
        {
            g_AnimState.currentVisibleHeight += g_AnimState.stepHeightPerFrame; 

            BOOL bFinished = FALSE;
            if (g_AnimState.isOpening) {
                if (g_AnimState.currentVisibleHeight >= g_AnimState.endHeight) bFinished = TRUE;
            } else {
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

        // 1. ���� ���۸��� ��-�޸� ��Ʈ�� �� Graphics ��ü ����
        Bitmap memBitmap(clientWidth, clientHeight);
        Graphics memGfx(&memBitmap);

        memGfx.SetTextRenderingHint(TextRenderingHintClearTypeGridFit); 
        memGfx.SetInterpolationMode(InterpolationModeHighQualityBicubic);
        memGfx.SetSmoothingMode(SmoothingModeAntiAlias);

        // 2. �޸𸮿� ��ü ��� �׸���
        SolidBrush bgBrush(Color(220, 40, 40, 45));
        memGfx.FillRectangle(&bgBrush, 0, 0, clientWidth, clientHeight);

        // 3. �޸𸮿� ������ �׸��� (����)
        int padding = 8;
        int itemHeight = 48;
        int iconSize = 32;
        int currentY = padding; // ��� �е�

        for (int i = 0; i < g_menuItems.size(); ++i)
        {
            // 3a. ������ ��Ʈ�ڽ�(RECT) ��� �� ����
            SetRect(&g_menuItems[i].rcItem, padding, currentY, clientWidth - padding, currentY + itemHeight);

            // 3b. ȣ�� ȿ�� �׸���
            if (i == g_hoverItem)
            {
                GraphicsPath path;
                RectF rcHoverF((REAL)g_menuItems[i].rcItem.left, (REAL)g_menuItems[i].rcItem.top, 
                               (REAL)(g_menuItems[i].rcItem.right - g_menuItems[i].rcItem.left), 
                               (REAL)(g_menuItems[i].rcItem.bottom - g_menuItems[i].rcItem.top));
                CreateRoundedRectPath(&path, rcHoverF, 4.0f); // 4px �ձ� �𼭸�
                memGfx.FillPath(g_brushHover.get(), &path);
            }

            // 3c. ������ �׸��� (�������� RECT �߾� ����)
            int iconPadding = (itemHeight - iconSize) / 2; // (48 - 32) / 2 = 8px
            int iconX = g_menuItems[i].rcItem.left + iconPadding;
            int iconY = g_menuItems[i].rcItem.top + iconPadding;
            
            // --- [������ ���� ����] ---
            if (g_menuItems[i].hIcon)
            {
                // 1. GDI+ Graphics ��ü���� GDI HDC�� ���ɴϴ�.
                HDC hMemDC = memGfx.GetHDC(); 

                // 2. ǥ�� Win32 GDI �Լ�(::DrawIconEx)�� ����Ͽ� HICON�� �� HDC�� �׸��ϴ�.
                ::DrawIconEx(
                    hMemDC, 
                    iconX, 
                    iconY, 
                    g_menuItems[i].hIcon, 
                    iconSize,  // 32
                    iconSize,  // 32
                    0, 
                    NULL, 
                    DI_NORMAL
                );

                // 3. ����� HDC�� ��� �����մϴ�.
                memGfx.ReleaseHDC(hMemDC);
            }
            // --- [���� ��] ---
            
            // 3d. �ؽ�Ʈ �׸���
            if (g_fontItem)
            {
                PointF textOrigin( (REAL)(iconX + iconSize + padding * 2), 
                                   (REAL)(g_menuItems[i].rcItem.top + (itemHeight / 2.0f)) );
                
                StringFormat format;
                format.SetAlignment(StringAlignmentNear);
                format.SetLineAlignment(StringAlignmentCenter); // ���� �߾� ����
                
                memGfx.DrawString(g_menuItems[i].text.c_str(), -1, g_fontItem.get(), textOrigin, &format, g_brushText.get());
            }

            currentY += itemHeight + padding; // ���� ������ Y ��ġ
        }

        // 4. ȭ�� HDC�� Graphics ��ü ����
        Graphics screenGfx(hdc);

        // 5. Ŭ���� ����(���̴� ����) ����
        RectF clipRect(0.0f, (REAL)clientHeight - (REAL)g_AnimState.currentVisibleHeight, 
                       (REAL)clientWidth, (REAL)g_AnimState.currentVisibleHeight);
        screenGfx.SetClip(clipRect);

        // 6. �ϼ��� �޸� ��Ʈ���� ȭ������ ���� (Ŭ�� ������ �׷���)
        screenGfx.DrawImage(&memBitmap, 0, 0);

        EndPaint(hWnd, &ps);
    }
    break;
    
    // --- [��Ŀ�� �𵨿� �Է� ó��] ---

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
        if (g_hoverItem != -1) // ��ȿ�� ������ ������ Ŭ��
        {
            LPCWSTR cmd = g_menuItems[g_hoverItem].command.c_str();
            HINSTANCE hRun = ShellExecuteW(hWnd, L"open", cmd, NULL, NULL, SW_SHOWNORMAL);
            
            if ((INT_PTR)hRun <= 32 && wcscmp(cmd, L"wt.exe") == 0) // wt.exe ���� ���� ��
            {
                 ShellExecuteW(hWnd, L"open", L"cmd.exe", NULL, NULL, SW_SHOWNORMAL); // cmd.exe�� ��ü
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

    case WM_KILLFOCUS: // ��Ŀ�� ���� (Click-away)
    {
        CloseMenuAndNotify(hWnd);
    }
    break;

    case WM_DESTROY:
    {
        for (auto& item : g_menuItems)
        {
            if (item.hIcon) DestroyIcon(item.hIcon);
        }
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
 * @brief ���� �޴� ������ Ŭ������ ����մϴ�.
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
 * @brief ���� �޴� �����츦 (������ ���·�) �����մϴ�.
 */
HWND StartMenu_Create(HINSTANCE hInstance, HWND hOwner)
{
    // [����] WS_EX_NOACTIVATE �÷��� ���� (��Ŀ���� Ŭ���� �޾ƾ� ��)
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


// --- [��Ŀ�� �𵨿� �ִϸ��̼� ����] ---

/**
 * @brief [����] ���� �޴��� Ŭ���� �ִϸ��̼ǰ� �Բ� ǥ���ϰ� "��Ŀ��"�� �����մϴ�.
 */
void StartMenu_Show(HWND hStartMenu, HWND hTaskbar)
{
    if (IsWindowVisible(hStartMenu) && g_AnimState.isOpening) {
        return;
    }
    if (g_AnimState.isAnimating) {
        KillTimer(hStartMenu, ID_TIMER_ANIMATE);
    }

    // 1. â�� ���� ��ġ�� ��� �̵���Ű�� ǥ��
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int finalTopY = screenHeight - 60 - MENU_HEIGHT;
    int finalX = (screenWidth - MENU_WIDTH) / 2;

    SetWindowPos(hStartMenu, HWND_TOPMOST, finalX, finalTopY, 0, 0, 
        SWP_NOSIZE | SWP_SHOWWINDOW); 

    // 2. [�ٽ�] â�� ��� ��Ŀ���� ����
    SetFocus(hStartMenu);

    // 3. �ִϸ��̼� ���� ���� ���� (Height: 0 -> MAX)
    g_AnimState.isAnimating = TRUE;
    g_AnimState.isOpening = TRUE;
    g_AnimState.currentVisibleHeight = 0.0f;
    g_AnimState.endHeight = (REAL)MENU_HEIGHT;
    
    float totalSteps = (float)(ANIMATION_DURATION_MS / ANIMATION_STEP_MS);
    g_AnimState.stepHeightPerFrame = ((REAL)MENU_HEIGHT / totalSteps);

    // 4. �ִϸ��̼� Ÿ�̸� ����
    SetTimer(hStartMenu, ID_TIMER_ANIMATE, ANIMATION_STEP_MS, NULL);
}

/**
 * @brief [����] ���� �޴��� Ŭ���� �ִϸ��̼ǰ� �Բ� ����ϴ�.
 */
void StartMenu_Hide(HWND hStartMenu)
{
    if (!IsWindowVisible(hStartMenu) && !g_AnimState.isOpening) {
        return;
    }
    if (g_AnimState.isAnimating) {
        KillTimer(hStartMenu, ID_TIMER_ANIMATE);
    }
    
    // 1. �ִϸ��̼� ���� ���� ���� (Height: MAX -> 0)
    g_AnimState.isAnimating = TRUE;
    g_AnimState.isOpening = FALSE;
    g_AnimState.currentVisibleHeight = (REAL)MENU_HEIGHT;
    g_AnimState.endHeight = 0.0f;

    float totalSteps = (float)(ANIMATION_DURATION_MS / ANIMATION_STEP_MS);
    g_AnimState.stepHeightPerFrame = -((REAL)MENU_HEIGHT / totalSteps);

    // 2. �ִϸ��̼� Ÿ�̸� ����
    SetTimer(hStartMenu, ID_TIMER_ANIMATE, ANIMATION_STEP_MS, NULL);
}
// --- [���� ��] ---