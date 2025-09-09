#pragma once

// 필요한 GDI+ 클래스를 전방 선언합니다.
// 이렇게 하면 헤더 파일에 GDI+ 전체를 포함할 필요가 없어지고,
// 컴파일러가 타입을 인식할 수 있게 됩니다.
namespace Gdiplus
{
    class GraphicsPath;
    struct RectF; // RectF는 구조체이므로 전방 선언이 가능합니다.
}

/**
 * @brief GraphicsPath 객체에 둥근 사각형 경로를 추가하는 헬퍼 함수입니다.
 * @param path (입출력) 경로를 추가할 GDI+ GraphicsPath 객체 포인터
 * @param rect (입력) 사각형의 위치와 크기 (RectF 사용)
 * @param cornerRadius (입력) 모서리의 반경
 */
void CreateRoundedRectPath(Gdiplus::GraphicsPath* path, Gdiplus::RectF rect, float cornerRadius);