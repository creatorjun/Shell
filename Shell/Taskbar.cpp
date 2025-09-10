// Taskbar.cpp : 작업표시줄(AppBar) 창 생성, 등록 및 메시지 처리를 구현합니다.
//

// windows.h 와 GDI+의 min/max 매크로 충돌을 방지합니다.
#define NOMINMAX

#include "framework.h"
#include "Taskbar.h"
#include "StartButton.h"
#include "StartMenu.h"
#include "Shell.h"
#include "Resource.h"
#include "Constants.h"

#include <shellapi.h>
#include <windowsx.h>
#include <memory>
#include <utility> // std::pair
#include <algorithm> // std::min, std::max

// --- [GDI+ 설정] ---
#include <Objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;
// --- [GDI+ 끝] ---

INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

namespace
{

    // --- [아크릴 효과 API 정의] ---
    enum ACCENT_STATE { ACCENT_DISABLED = 0, ACCENT_ENABLE_ACRYLICBLURBEHIND = 4 };
    struct ACCENT_POLICY { ACCENT_STATE AccentState; DWORD AccentFlags; DWORD GradientColor; DWORD AnimationId; };
    struct WINDOWCOMPOSITIONATTRIBDATA { int Attrib; PVOID pvData; SIZE_T cbData; };
    using pSetWindowCompositionAttribute = BOOL(WINAPI*)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);

    // --- [내부 데이터 구조체] ---
    struct TASKBAR_DATA
    {
        std::unique_ptr<Gdiplus::Image> imgBackground;
        std::unique_ptr<Gdiplus::Image> imgStart;
        std::unique_ptr<Gdiplus::TextureBrush> brushBackground;

        Gdiplus::Color gradientColorLeft;
        Gdiplus::Color gradientColorRight;

        BOOL bStartHover;
        BOOL bMouseTracking;
        BOOL bStartActive;
        HWND hStartMenu;

        TASKBAR_DATA() :
            imgBackground(nullptr),
            imgStart(nullptr),
            brushBackground(nullptr),
            gradientColorLeft(ACRYLIC_EFFECT_ALPHA, ACRYLIC_EFFECT_RED, ACRYLIC_EFFECT_GREEN, ACRYLIC_EFFECT_BLUE),
            gradientColorRight(ACRYLIC_EFFECT_ALPHA, ACRYLIC_EFFECT_RED, ACRYLIC_EFFECT_GREEN, ACRYLIC_EFFECT_BLUE),
            bStartHover(FALSE),
            bMouseTracking(FALSE),
            bStartActive(FALSE),
            hStartMenu(NULL)
        {
        }
    };

    static HINSTANCE g_hInstTaskbar;

    // --- [내부 헬퍼 함수] ---
    float HueToRgb(float p, float q, float t)
    {
        if (t < 0.0f) t += 1.0f;
        if (t > 1.0f) t -= 1.0f;
        if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
        if (t < 1.0f / 2.0f) return q;
        if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
        return p;
    }

    void HslToRgb(float h, float s, float l, BYTE* r, BYTE* g, BYTE* b)
    {
        if (s == 0.0f) {
            *r = *g = *b = static_cast<BYTE>(l * 255);
        }
        else {
            float q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
            float p = 2.0f * l - q;
            *r = static_cast<BYTE>(HueToRgb(p, q, h + 1.0f / 3.0f) * 255);
            *g = static_cast<BYTE>(HueToRgb(p, q, h) * 255);
            *b = static_cast<BYTE>(HueToRgb(p, q, h - 1.0f / 3.0f) * 255);
        }
    }

    void RgbToHsl(BYTE r, BYTE g, BYTE b, float* h, float* s, float* l)
    {
        float rf = r / 255.0f;
        float gf = g / 255.0f;
        float bf = b / 255.0f;
        float maxVal = std::max({ rf, gf, bf });
        float minVal = std::min({ rf, gf, bf });
        *h = *s = 0;
        *l = (maxVal + minVal) / 2.0f;
        if (maxVal != minVal) {
            float d = maxVal - minVal;
            *s = *l > 0.5f ? d / (2.0f - maxVal - minVal) : d / (maxVal + minVal);
            if (maxVal == rf) *h = (gf - bf) / d + (gf < bf ? 6.0f : 0.0f);
            else if (maxVal == gf) *h = (bf - rf) / d + 2.0f;
            else *h = (rf - gf) / d + 4.0f;
            *h /= 6.0f;
        }
    }

    void EnableAcrylicEffect(HWND hWnd, Gdiplus::Color color)
    {
        HMODULE hUser32 = GetModuleHandle(L"user32.dll");
        if (hUser32)
        {
            pSetWindowCompositionAttribute SetWindowCompositionAttribute =
                (pSetWindowCompositionAttribute)GetProcAddress(hUser32, "SetWindowCompositionAttribute");
            if (SetWindowCompositionAttribute)
            {
                DWORD gradientColor = (color.GetA() << 24) | (color.GetB() << 16) | (color.GetG() << 8) | (color.GetR());
                ACCENT_POLICY accent = { ACCENT_ENABLE_ACRYLICBLURBEHIND, 0, gradientColor, 0 };
                WINDOWCOMPOSITIONATTRIBDATA data = { 19, &accent, sizeof(accent) };
                SetWindowCompositionAttribute(hWnd, &data);
            }
        }
    }

    std::pair<Gdiplus::Color, Gdiplus::Color> GetWallpaperGradientColors()
    {
        WCHAR wallpaperPath[MAX_PATH];
        Color defaultColor(ACRYLIC_EFFECT_ALPHA, ACRYLIC_EFFECT_RED, ACRYLIC_EFFECT_GREEN, ACRYLIC_EFFECT_BLUE);

        if (SystemParametersInfoW(SPI_GETDESKWALLPAPER, MAX_PATH, wallpaperPath, 0))
        {
            std::unique_ptr<Bitmap> wallpaperBitmap(Bitmap::FromFile(wallpaperPath));
            if (wallpaperBitmap && wallpaperBitmap->GetLastStatus() == Ok)
            {
                UINT width = wallpaperBitmap->GetWidth();
                UINT height = wallpaperBitmap->GetHeight();
                Rect rectLeft(0, height - 10, 1, 10);
                Rect rectRight(width - 1, height - 10, 1, 10);

                Bitmap bmpLeft(1, 1);
                Graphics gLeft(&bmpLeft);
                gLeft.SetInterpolationMode(InterpolationModeHighQualityBicubic);
                gLeft.DrawImage(wallpaperBitmap.get(), Rect(0, 0, 1, 1), rectLeft.X, rectLeft.Y, rectLeft.Width, rectLeft.Height, UnitPixel);

                Bitmap bmpRight(1, 1);
                Graphics gRight(&bmpRight);
                gRight.SetInterpolationMode(InterpolationModeHighQualityBicubic);
                gRight.DrawImage(wallpaperBitmap.get(), Rect(0, 0, 1, 1), rectRight.X, rectRight.Y, rectRight.Width, rectRight.Height, UnitPixel);

                Color colorLeft, colorRight;
                bmpLeft.GetPixel(0, 0, &colorLeft);
                bmpRight.GetPixel(0, 0, &colorRight);

                float h, s, l;
                RgbToHsl(colorLeft.GetR(), colorLeft.GetG(), colorLeft.GetB(), &h, &s, &l);
                l = std::max(0.20f, l); s = std::max(0.10f, s);
                BYTE rL, gL, bL;
                HslToRgb(h, s, l, &rL, &gL, &bL);

                RgbToHsl(colorRight.GetR(), colorRight.GetG(), colorRight.GetB(), &h, &s, &l);
                l = std::max(0.20f, l); s = std::max(0.10f, s);
                BYTE rR, gR, bR;
                HslToRgb(h, s, l, &rR, &gR, &bR);

                return { Color(ACRYLIC_EFFECT_ALPHA, rL, gL, bL), Color(ACRYLIC_EFFECT_ALPHA, rR, gR, bR) };
            }
        }
        return { defaultColor, defaultColor };
    }

    void GetStartButtonRect(HWND hWnd, RECT* pButtonRect)
    {
        if (!pButtonRect) return;
        RECT rcClient;
        GetClientRect(hWnd, &rcClient);
        int clientWidth = rcClient.right - rcClient.left;
        int clientHeight = rcClient.bottom - rcClient.top;
        int hoverX = (clientWidth - START_BUTTON_HOVER_SIZE) / 2;
        int hoverY = (clientHeight - START_BUTTON_HOVER_SIZE) / 2;
        SetRect(pButtonRect, hoverX, hoverY, hoverX + START_BUTTON_HOVER_SIZE, hoverY + START_BUTTON_HOVER_SIZE);
    }

    LRESULT CALLBACK Taskbar_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (message == WM_APP_MENU_CLOSED)
        {
            TASKBAR_DATA* pData = (TASKBAR_DATA*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
            if (pData && pData->bStartActive)
            {
                pData->bStartActive = FALSE;
                POINT ptCursor;
                GetCursorPos(&ptCursor);
                ScreenToClient(hWnd, &ptCursor);
                RECT rcButton;
                GetStartButtonRect(hWnd, &rcButton);
                BOOL bIsOverButton = PtInRect(&rcButton, ptCursor);
                if (bIsOverButton != pData->bStartHover) pData->bStartHover = bIsOverButton;
                InvalidateRect(hWnd, &rcButton, FALSE);
            }
            return 0;
        }

        switch (message)
        {
        case WM_CREATE:
        {
            CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
            TASKBAR_DATA* pData = (TASKBAR_DATA*)pCreate->lpCreateParams;
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pData);

            if (GRADIENT_EFFECT_ENABLED && pData)
            {
                auto colors = GetWallpaperGradientColors();
                pData->gradientColorLeft = colors.first;
                pData->gradientColorRight = colors.second;
            }
            break;
        }
        case WM_SETTINGCHANGE:
        {
            if (wParam == SPI_SETDESKWALLPAPER)
            {
                TASKBAR_DATA* pData = (TASKBAR_DATA*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
                if (GRADIENT_EFFECT_ENABLED && pData)
                {
                    auto colors = GetWallpaperGradientColors();
                    pData->gradientColorLeft = colors.first;
                    pData->gradientColorRight = colors.second;
                    InvalidateRect(hWnd, NULL, TRUE);
                }
            }
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
            break;
        }
        case WM_PAINT:
        {
            TASKBAR_DATA* pData = (TASKBAR_DATA*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            RECT rcClient;
            GetClientRect(hWnd, &rcClient);
            int clientWidth = rcClient.right - rcClient.left;
            int clientHeight = rcClient.bottom - rcClient.top;

            Bitmap memBitmap(clientWidth, clientHeight);
            Graphics* memGfx = Graphics::FromImage(&memBitmap);

            if (GRADIENT_EFFECT_ENABLED && pData)
            {
                Color opaqueLeft(255, pData->gradientColorLeft.GetR(), pData->gradientColorLeft.GetG(), pData->gradientColorLeft.GetB());
                Color opaqueRight(255, pData->gradientColorRight.GetR(), pData->gradientColorRight.GetG(), pData->gradientColorRight.GetB());

                LinearGradientBrush gradBrush(
                    Point(0, 0), Point(clientWidth, 0),
                    opaqueLeft, opaqueRight);
                memGfx->FillRectangle(&gradBrush, 0, 0, clientWidth, clientHeight);

                // [추가] 엠보싱 효과를 위한 하이라이트 및 그림자
                Pen highlightPen(Color(80, 255, 255, 255), 1);
                Pen shadowPen(Color(80, 0, 0, 0), 1);
                memGfx->DrawLine(&highlightPen, 0, 0, clientWidth, 0);
                memGfx->DrawLine(&shadowPen, 0, clientHeight - 1, clientWidth, clientHeight - 1);
            }
            else if (pData && pData->brushBackground)
            {
                memGfx->FillRectangle(pData->brushBackground.get(), 0, 0, clientWidth, clientHeight);
            }
            else
            {
                SolidBrush fallbackBrush(Color(255, 0, 0, 0));
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
            break;
        }
        case WM_MOUSEMOVE:
        {
            TASKBAR_DATA* pData = (TASKBAR_DATA*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
            if (pData)
            {
                if (!pData->bMouseTracking)
                {
                    TRACKMOUSEEVENT tme = { sizeof(tme) };
                    tme.dwFlags = TME_LEAVE;
                    tme.hwndTrack = hWnd;
                    if (TrackMouseEvent(&tme)) pData->bMouseTracking = TRUE;
                }
                POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                RECT rcButton;
                GetStartButtonRect(hWnd, &rcButton);
                BOOL bIsOverButton = PtInRect(&rcButton, pt);
                if (bIsOverButton != pData->bStartHover)
                {
                    pData->bStartHover = bIsOverButton;
                    if (!pData->bStartActive) InvalidateRect(hWnd, &rcButton, FALSE);
                }
            }
            break;
        }
        case WM_MOUSELEAVE:
        {
            TASKBAR_DATA* pData = (TASKBAR_DATA*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
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
            TASKBAR_DATA* pData = (TASKBAR_DATA*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
            if (pData && pData->hStartMenu)
            {
                POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                RECT rcButton;
                GetStartButtonRect(hWnd, &rcButton);
                if (PtInRect(&rcButton, pt))
                {
                    pData->bStartActive = !pData->bStartActive;
                    if (pData->bStartActive) StartMenu::Show(pData->hStartMenu, hWnd);
                    else StartMenu::Hide(pData->hStartMenu);
                    InvalidateRect(hWnd, &rcButton, FALSE);
                }
                else
                {
                    if (pData->bStartActive)
                    {
                        pData->bStartActive = FALSE;
                        StartMenu::Hide(pData->hStartMenu);
                        InvalidateRect(hWnd, &rcButton, FALSE);
                    }
                }
            }
            break;
        }
        case WM_RBUTTONDOWN:
        {
            TASKBAR_DATA* pData = (TASKBAR_DATA*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
            if (pData && pData->bStartActive)
            {
                pData->bStartActive = FALSE;
                StartMenu::Hide(pData->hStartMenu);
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

            TASKBAR_DATA* pData = (TASKBAR_DATA*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
            if (pData)
            {
                if (pData->hStartMenu) DestroyWindow(pData->hStartMenu);
                delete pData;
            }
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)NULL);
            PostQuitMessage(0);
            break;
        }
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        return 0;
    }

} // 익명 네임스페이스 끝

ATOM Taskbar::Register(HINSTANCE hInstance, const WCHAR* szWindowClass)
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

BOOL Taskbar::Create(HINSTANCE hInst, int nCmdShow,
    const WCHAR* szTitle, const WCHAR* szWindowClass,
    std::unique_ptr<Gdiplus::Image> bg, std::unique_ptr<Gdiplus::Image> start)
{
    g_hInstTaskbar = hInst;
    TASKBAR_DATA* pData = new TASKBAR_DATA();
    if (!pData) return FALSE;

    pData->imgBackground = std::move(bg);
    pData->imgStart = std::move(start);
    if (pData->imgBackground)
    {
        pData->brushBackground = std::make_unique<TextureBrush>(pData->imgBackground.get());
    }

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    HWND hWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW, szWindowClass, szTitle, WS_POPUP,
        0, 0, screenWidth, TASKBAR_HEIGHT,
        nullptr, nullptr, hInst, (LPVOID)pData
    );
    if (!hWnd)
    {
        delete pData;
        return FALSE;
    }

    pData->hStartMenu = StartMenu::Create(hInst, hWnd);
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
    abd.rc.top = screenHeight - TASKBAR_HEIGHT;
    abd.rc.bottom = screenHeight;
    SHAppBarMessage(ABM_QUERYPOS, &abd);
    MoveWindow(hWnd, abd.rc.left, abd.rc.top, abd.rc.right - abd.rc.left, abd.rc.bottom - abd.rc.top, TRUE);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return TRUE;
}