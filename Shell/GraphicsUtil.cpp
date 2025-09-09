// [추가] windows.h 와 GDI+의 min/max 매크로 충돌을 방지합니다.
#define NOMINMAX

#include "framework.h"
#include "GraphicsUtil.h"

// [추가] GDI+가 의존하는 헤더들을 포함합니다.
#include <objidl.h>

// [추가] GDI+의 전체 정의를 사용하기 위해 cpp 파일에서 직접 포함합니다.
#include <gdiplus.h>

/**
 * @brief GraphicsPath 객체에 둥근 사각형 경로를 추가하는 헬퍼 함수입니다.
 * @param path (입출력) 경로를 추가할 GDI+ GraphicsPath 객체 포인터
 * @param rect (입력) 사각형의 위치와 크기 (RectF 사용)
 * @param cornerRadius (입력) 모서리의 반경
 */
void CreateRoundedRectPath(Gdiplus::GraphicsPath* path, Gdiplus::RectF rect, float cornerRadius)
{
    if (!path) return;

    // 모서리 반경의 2배 (지름)
    float dia = cornerRadius * 2.0f;
    if (dia <= 0.0f)
    {
        // 반경이 0이면 단순 사각형 추가
        path->AddRectangle(rect);
        return;
    }

    // GDI+는 4개의 호와 4개의 선을 직접 연결해야 둥근 사각형이 됩니다.
    // (시계 방향: 좌상단 -> 우상단 -> 우하단 -> 좌하단)
    path->AddArc(rect.X, rect.Y, dia, dia, 180, 90); // 좌상단 호
    path->AddArc(rect.GetRight() - dia, rect.Y, dia, dia, 270, 90); // 우상단 호
    path->AddArc(rect.GetRight() - dia, rect.GetBottom() - dia, dia, dia, 0, 90); // 우하단 호
    path->AddArc(rect.X, rect.GetBottom() - dia, dia, dia, 90, 90); // 좌하단 호

    // 경로를 닫아 도형을 완성합니다.
    path->CloseFigure();
}