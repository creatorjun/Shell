// Clock.cpp : �ð� ǥ�� ���� ����� �����մϴ�.
//

#define NOMINMAX

#include "framework.h"
#include "Clock.h"
#include <memory>
#include <string>

// --- [GDI+ ����] ---
#include <Objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;
// --- [GDI+ ��] ---

namespace
{
    // Clock ��� ���ο����� ����� GDI+ ��ü
    std::unique_ptr<Gdiplus::Font> g_fontClock;
    std::unique_ptr<Gdiplus::SolidBrush> g_brushClock;
}

BOOL Clock::Initialize()
{
    // �ð� ��Ʈ�� �귯�� ����
    g_fontClock.reset(new Font(L"���� ���", 9.0f, FontStyleRegular, UnitPoint));
    g_brushClock.reset(new SolidBrush(Color(255, 255, 255, 255)));

    // ��ü ���� ���� ���� Ȯ��
    if (!g_fontClock || !g_brushClock)
    {
        return FALSE;
    }
    return TRUE;
}

void Clock::Shutdown()
{
    // unique_ptr�� �ڵ����� �޸𸮸� �����ϹǷ� reset() ȣ��� ���
    g_fontClock.reset();
    g_brushClock.reset();
}

void Clock::FormatTime(WCHAR* pszBuffer, size_t cchBuffer)
{
    SYSTEMTIME st;
    GetLocalTime(&st);

    // ����/���� ���ڿ� �� 12�ð��� �ð� ���
    WCHAR szAmPm[3];
    int hour = st.wHour;
    if (hour >= 12) {
        wcscpy_s(szAmPm, L"����");
        if (hour > 12) hour -= 12;
    }
    else {
        wcscpy_s(szAmPm, L"����");
        if (hour == 0) hour = 12;
    }

    // ���� ���ڿ� ����ȭ
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
    strFormat.SetAlignment(StringAlignmentFar);      // ���� ����: ������
    strFormat.SetLineAlignment(StringAlignmentCenter); // ���� ����: ���

    // �׸��� ������ �۾� ǥ���� ���� ������ ���� (������ �е� 15px)
    RectF clockRect((REAL)rcClient.left, (REAL)rcClient.top, (REAL)clientWidth - 15, (REAL)clientHeight);

    // GDI+�� ����Ͽ� �ð� ���ڿ� �׸���
    pGfx->DrawString(pszTime, -1, g_fontClock.get(), clockRect, &strFormat, g_brushClock.get());
}