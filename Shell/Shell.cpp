// creatorjun/shell/Shell-04869ccf080a38ea887a3d00139afecc32776daa/Shell/Shell.cpp

// Shell.cpp : 애플리케이션 진입점 및 모듈 로더 역할을 합니다.
//

// windows.h 와 GDI+의 min/max 매크로 충돌을 방지합니다.
#define NOMINMAX

// --- 기존 코드 ---
#include "framework.h"
#include "Shell.h"
#include <shellapi.h>           // Shell API(AppBar)

// --- [GDI+ 설정] ---
#include <Objidl.h>             // IStream, CreateStreamOnHGlobal
#include <gdiplus.h>            // GDI+
using namespace Gdiplus;        // GDI+ 네임스페이스

#pragma comment(lib, "Gdiplus.lib") // GDI+ 라이브러리 링크
#pragma comment(lib, "Ole32.lib")   // COM 스트림(CreateStreamOnHGlobal) 링크
// --- [GDI+ 끝] ---

// --- [새 모듈 포함] ---
// (이 파일들은 앞으로 생성할 파일입니다)
#include "Desktop.h"
#include "Taskbar.h"
#include "StartMenu.h"          // [추가] 시작 메뉴 모듈 헤더 포함
// --- [모듈 포함 끝] ---


#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// --- [GDI+ 전역 객체] ---
// (모듈 로딩을 위해 wWinMain에서 관리)
ULONG_PTR gdiplusToken;
Image* g_imgBackground = nullptr;
Image* g_imgStart = nullptr;

// 함수 선언 (정보 상자는 여기에 남겨둡니다)
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

// --- [GDI+ 리소스 로더 함수] ---
// (이 헬퍼 함수는 여러 모듈에서 필요할 수 있으나, 
//  여기서는 진입점에서 이미지를 미리 로드하는 데 사용됩니다.)
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


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    hInst = hInstance; // 전역 인스턴스 저장

    // --- [GDI+ 초기화] ---
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    // --- ---

    // 1. [수정] 이미지를 윈도우 생성(WM_CREATE) 전, 메인에서 미리 로드합니다.
    g_imgBackground = LoadImageFromResource(hInst, IDR_BG_JPG, L"JPEG");
    g_imgStart = LoadImageFromResource(hInst, IDR_START_PNG, L"PNG");

    // 2. [모듈 호출] 바탕화면(Desktop) 모듈 초기화 (배경화면 설정)
    if (g_imgBackground)
    {
        Desktop_SetWallpaper(g_imgBackground);
    }

    // 3. [모듈 호출] 작업표시줄(Taskbar) 및 시작메뉴(StartMenu) 모듈 초기화 (창 클래스 등록)
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SHELL, szWindowClass, MAX_LOADSTRING);

    // Taskbar 모듈에 창 클래스 이름과 등록을 위임합니다.
    Taskbar_Register(hInstance, szWindowClass);

    // --- [추가] 시작 메뉴 모듈의 창 클래스를 등록합니다. ---
    StartMenu_Register(hInstance);
    // --- [추가 끝] ---


    // 4. [모듈 호출] 작업표시줄(Taskbar) 생성
    //    (Taskbar 모듈이 창을 생성하고, 그림 그리기에 필요한 이미지 포인터를 전달받습니다)
    if (!Taskbar_Create(hInstance, nCmdShow, szTitle, szWindowClass, g_imgBackground, g_imgStart))
    {
        return FALSE;
    }

    MSG msg;

    // 5. 기본 메시지 루프입니다:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // --- [GDI+ 및 리소스 해제] ---
    // 메인에서 로드했던 이미지 리소스를 해제합니다.
    if (g_imgBackground) { delete g_imgBackground; }
    if (g_imgStart) { delete g_imgStart; }

    GdiplusShutdown(gdiplusToken);
    // --- ---

    return (int)msg.wParam;
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