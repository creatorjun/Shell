// Shell.cpp

// windows.h 와 GDI+의 min/max 매크로 충돌을 방지합니다.
#define NOMINMAX

// --- 기존 코드 ---
#include "framework.h"
#include "Shell.h"
#include <shellapi.h>               // Shell API(AppBar)

// --- [GDI+ 설정] ---
#include <Objidl.h>                 // IStream, CreateStreamOnHGlobal
#include <gdiplus.h>                // GDI+
using namespace Gdiplus;            // GDI+ 네임스페이스

#pragma comment(lib, "Gdiplus.lib") // GDI+ 라이브러리 링크
#pragma comment(lib, "Ole32.lib")   // COM 스트림(CreateStreamOnHGlobal) 링크
// --- [GDI+ 끝] ---

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                         // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];           // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];     // 기본 창 클래스 이름입니다.

// --- [GDI+ 전역 객체 추가] ---
ULONG_PTR gdiplusToken;                  // GDI+ 토큰
Image* g_imgBackground = nullptr;        // 배경 이미지 포인터
Image* g_imgStart = nullptr;             // 시작 버튼 이미지 포인터

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// --- [GDI+ 리소스 로더 함수] ---
// 리소스에서 이미지를 로드하는 헬퍼 함수
Image* LoadImageFromResource(HINSTANCE hInst, UINT resourceId, const WCHAR* resourceType)
{
    HRSRC hRes = FindResource(hInst, MAKEINTRESOURCE(resourceId), resourceType);
    if (!hRes) return nullptr;
    DWORD resSize = SizeofResource(hInst, hRes);
    HGLOBAL hResData = LoadResource(hInst, hRes);
    if (!hResData) return nullptr;
    void* pResData = LockResource(hResData);
    if (!pResData) return nullptr;

    IStream* pStream = nullptr;
    if (CreateStreamOnHGlobal(NULL, TRUE, &pStream) == S_OK)
    {
        ULONG written = 0;
        if (SUCCEEDED(pStream->Write(pResData, resSize, &written)))
        {
            LARGE_INTEGER liZero = {};
            ULARGE_INTEGER newPos = {};
            pStream->Seek(liZero, STREAM_SEEK_SET, &newPos); // 스트림 포지션 0으로 되감기

            Image* pImage = Image::FromStream(pStream);
            pStream->Release();
            return pImage;
        }
        pStream->Release();
    }
    return nullptr;
}

// GDI+ 이미지 인코더 CLSID를 찾는 헬퍼 함수
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT  num = 0;          // number of image encoders
    UINT  size = 0;         // size of the image encoder array in bytes

    ImageCodecInfo* pImageCodecInfo = NULL;

    GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1;  // Failure

    pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL)
        return -1;  // Failure

    GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j)
    {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;  // Success
        }
    }

    free(pImageCodecInfo);
    return -1;  // Failure
}
// [[[ 함수 추가 끝 ]]]

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // --- [GDI+ 초기화] ---
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    // --- ---

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

    // 기본 메시지 루프입니다:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // --- [GDI+ 종료] ---
    GdiplusShutdown(gdiplusToken);
    // --- ---

    return (int)msg.wParam;
}

// 함수: MyRegisterClass() - (수정 없음)
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
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

// 함수: InitInstance(HINSTANCE, int)
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

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
        hInstance,
        nullptr
    );

    if (!hWnd)
    {
        return FALSE;
    }

    APPBARDATA abd = {};
    abd.cbSize = sizeof(APPBARDATA);
    abd.hWnd = hWnd;

    // 1. 새 AppBar로 시스템에 등록합니다.
    if (!SHAppBarMessage(ABM_NEW, &abd))
    {
        return FALSE;
    }

    // 2. AppBar 위치 및 크기 정의 (화면 하단, 48px)
    abd.uEdge = ABE_BOTTOM;
    abd.rc.left = 0;
    abd.rc.right = screenWidth;
    abd.rc.top = screenHeight - barHeight;
    abd.rc.bottom = screenHeight;

    // 3. 시스템에 해당 공간을 예약하도록 요청 (QUERYPOS)
    SHAppBarMessage(ABM_QUERYPOS, &abd);

    // 4. [주석 처리됨 - 피드백 반영]
    // abd.rc.top = abd.rc.bottom - barHeight;

    // 5. [주석 처리됨 - 피드백 반영]
     //SHAppBarMessage(ABM_SETPOS, &abd);

    // 6. 우리 창을 시스템이 지정한 최종 위치와 크기로 이동시킵니다.
    MoveWindow(hWnd, abd.rc.left, abd.rc.top, abd.rc.right - abd.rc.left, abd.rc.bottom - abd.rc.top, TRUE);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

// 함수: WndProc(HWND, UINT, WPARAM, LPARAM)
// 용도: 주 창의 메시지를 처리합니다. (WM_CREATE, WM_PAINT, WM_DESTROY 수정됨)
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        // [수정] 배경화면 설정 로직 추가
    case WM_CREATE:
    {
        // 1. 리소스에서 이미지 로드 (기존 코드)
        g_imgBackground = LoadImageFromResource(hInst, IDR_BG_JPG, L"JPEG");
        g_imgStart = LoadImageFromResource(hInst, IDR_START_PNG, L"PNG");

        // --- [[[ 2. 바탕화면 설정 로직 추가 ]]] ---
        if (g_imgBackground)
        {
            CLSID jpgClsid;
            WCHAR tempPath[MAX_PATH];
            WCHAR jpgTempFilePath[MAX_PATH];

            // 2a. JPEG 인코더 CLSID 가져오기
            if (GetEncoderClsid(L"image/jpeg", &jpgClsid) != -1)
            {
                // 2b. Windows 임시 폴더 경로 가져오기
                if (GetTempPathW(MAX_PATH, tempPath))
                {
                    // 2c. 임시 파일 경로 생성 (예: C:\Users\User\AppData\Local\Temp\MyShellBG.jpg)
                    swprintf_s(jpgTempFilePath, MAX_PATH, L"%sMyShellBG.jpg", tempPath);

                    // 2d. GDI+ Image 객체를 실제 파일로 저장
                    if (g_imgBackground->Save(jpgTempFilePath, &jpgClsid, NULL) == Ok)
                    {
                        // 2e. Windows API를 호출하여 바탕화면 변경 및 시스템에 알림
                        SystemParametersInfoW(
                            SPI_SETDESKWALLPAPER,
                            0,
                            (PVOID)jpgTempFilePath,
                            SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE
                        );
                    }
                }
            }
        }
        // --- [[[ 로직 추가 끝 ]]] ---

        break; // DefWindowProc로 넘겨 윈도우 생성을 완료해야 합니다.
    }
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

    // [수정] GDI+를 사용하여 배경과 시작 버튼을 그립니다.
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // GDI+ 그래픽스 객체 생성
        Graphics graphics(hdc);

        // 클라이언트 영역(작업표시줄 창) 크기 가져오기
        RECT rcClient;
        GetClientRect(hWnd, &rcClient);
        int clientWidth = rcClient.right - rcClient.left;
        int clientHeight = rcClient.bottom - rcClient.top;

        // 1. 배경 그리기 (Background.jpg) - TextureBrush로 타일링
        if (g_imgBackground)
        {
            TextureBrush tBrush(g_imgBackground);
            graphics.FillRectangle(&tBrush, 0, 0, clientWidth, clientHeight);
        }
        else
        {
            // 이미지 로드 실패 시 단색 채우기
            FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
        }

        // 2. 시작 버튼 그리기 (Start.png) - 32px 정사각 크기로 가운데 정렬
        if (g_imgStart)
        {
            const int targetSize = 32; // px (필요 시 DPI 스케일 적용)
            int posX = (clientWidth - targetSize) / 2;
            int posY = (clientHeight - targetSize) / 2;
            graphics.DrawImage(g_imgStart, posX, posY, targetSize, targetSize);
        }

        EndPaint(hWnd, &ps);
    }
    break;

    // [수정] GDI+ 이미지 객체들을 메모리에서 해제합니다.
    case WM_DESTROY:
    {
        // --- AppBar 등록 해제 ---
        APPBARDATA abd = {};
        abd.cbSize = sizeof(APPBARDATA);
        abd.hWnd = hWnd;
        SHAppBarMessage(ABM_REMOVE, &abd);
        // --- ---

        // --- GDI+ 리소스 해제 ---
        if (g_imgBackground) { delete g_imgBackground; g_imgBackground = nullptr; }
        if (g_imgStart) { delete g_imgStart;      g_imgStart = nullptr; }
        // --- ---

        PostQuitMessage(0);
    }
    break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다. (수정 없음)
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
