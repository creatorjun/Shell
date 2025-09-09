#pragma once

// 필요한 GDI+ 클래스를 전방 선언합니다.
namespace Gdiplus
{
    class GraphicsPath;
    // [수정] RectF를 struct 대신 class로 전방 선언하여 경고 C4099를 해결합니다.
    class RectF;
}

/**
 * @brief GraphicsPath 객체에 둥근 사각형 경로를 추가하는 헬퍼 함수입니다.
 * @param path (입출력) 경로를 추가할 GDI+ GraphicsPath 객체 포인터
 * @param rect (입력) 사각형의 위치와 크기 (RectF 사용)
 * @param cornerRadius (입력) 모서리의 반경
 */
void CreateRoundedRectPath(Gdiplus::GraphicsPath* path, Gdiplus::RectF rect, float cornerRadius);