#pragma once
#include <windows.h>

// GDI+ 클래스 전방 선언
namespace Gdiplus
{
    class Graphics;
}

namespace Clock
{
    /**
     * @brief 시계 표시에 필요한 GDI+ 리소스(폰트, 브러시)를 초기화합니다.
     * @return 성공 시 TRUE, 실패 시 FALSE
     */
    BOOL Initialize();

    /**
     * @brief 초기화했던 GDI+ 리소스를 해제합니다.
     */
    void Shutdown();

    /**
     * @brief 현재 시간과 날짜를 형식에 맞는 문자열로 포맷합니다.
     * @param pszBuffer [out] 포맷된 문자열을 저장할 버퍼
     * @param cchBuffer [in] 버퍼의 크기 (문자 단위)
     */
    void FormatTime(WCHAR* pszBuffer, size_t cchBuffer);

    /**
     * @brief 제공된 Graphics 객체 위에 시간 문자열을 그립니다.
     * @param pGfx - 그림을 그릴 GDI+ Graphics 객체 포인터
     * @param rcClient - 그리기의 기준이 될 클라이언트 영역 RECT
     * @param pszTime - 화면에 표시할 시간 문자열
     */
    void Paint(Gdiplus::Graphics* pGfx, const RECT& rcClient, const WCHAR* pszTime);
}