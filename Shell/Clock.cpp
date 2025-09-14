// Clock.cpp : 시계 표시 관련 기능을 구현합니다.
//

#define NOMINMAX

#include "framework.h"
#include "Clock.h"
#include <memory>
#include <string>

// --- [GDI+ 설정] ---
#include <Objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;
// --- [GDI+ 끝] ---

namespace
{
    // Clock 모듈 내부에서만 사용할 GDI+ 객체
    std::unique_ptr<Gdiplus::Font> g_fontClock;
    std::unique_ptr<Gdiplus::SolidBrush> g_brushClock;
}

BOOL Clock::Initialize()
{
    // 시계 폰트와 브러시 생성
    g_fontClock.reset(new Font(L"맑은 고딕", 9.0f, FontStyleRegular, UnitPoint));
    g_brushClock.reset(new SolidBrush(Color(255, 255, 255, 255)));

    // 객체 생성 성공 여부 확인
    if (!g_fontClock || !g_brushClock)
    {
        return FALSE;
    }
    return TRUE;
}

void Clock::Shutdown()
{
    // unique_ptr가 자동으로 메모리를 해제하므로 reset() 호출로 충분
    g_fontClock.reset();
    g_brushClock.reset();
}

void Clock::FormatTime(WCHAR* pszBuffer, size_t cchBuffer)
{
    SYSTEMTIME st;
    GetLocalTime(&st);

    // 오전/오후 문자열 및 12시간제 시간 계산
    WCHAR szAmPm[3];
    int hour = st.wHour;
    if (hour >= 12) {
        wcscpy_s(szAmPm, L"오후");
        if (hour > 12) hour -= 12;
    }
    else {
        wcscpy_s(szAmPm, L"오전");
        if (hour == 0) hour = 12;
    }

    // 최종 문자열 형식화
    wsprintfW(pszBuffer, L"%04d-%02d-%02d\n%s %d:%02d",
        st.wYear, st.wMonth, st.wDay,
        szAmPm, hour, st.wMinute);
}

void Clock::Paint(Gdiplus::Graphics* pGfx, const RECT& rcClient, const WCHAR* pszTime)
{
    if (!pGfx || !g_fontClock || !g_brushClock)
    {
        return;
    }

    int clientWidth = rcClient.right - rcClient.left;
    int clientHeight = rcClient.bottom - rcClient.top;

    StringFormat strFormat;
    strFormat.SetAlignment(StringAlignmentFar);      // 가로 정렬: 오른쪽
    strFormat.SetLineAlignment(StringAlignmentCenter); // 세로 정렬: 가운데

    // 그리기 영역을 작업 표시줄 우측 끝으로 지정 (오른쪽 패딩 15px)
    RectF clockRect((REAL)rcClient.left, (REAL)rcClient.top, (REAL)clientWidth - 15, (REAL)clientHeight);

    // GDI+를 사용하여 시간 문자열 그리기
    pGfx->DrawString(pszTime, -1, g_fontClock.get(), clockRect, &strFormat, g_brushClock.get());
}