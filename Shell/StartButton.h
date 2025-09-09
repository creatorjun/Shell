// creatorjun/shell/Shell-04869ccf080a38ea887a3d00139afecc32776daa/Shell/StartButton.h

#pragma once
#include <windows.h> // RECT 타입을 사용하기 위해 포함

// GDI+ 클래스 전방 선언
namespace Gdiplus
{
    class Graphics;
    class Image;
}

/**
 * @brief 작업표시줄 위에 시작 버튼 이미지를 그립니다.
 * @param gfx - 그리기 대상인 작업표시줄의 GDI+ Graphics 객체 포인터
 * @param taskbarRect - 작업표시줄 창의 전체 RECT
 * @param imgStart - 리소스에서 로드된 시작 버튼 GDI+ Image 객체 포인터
 * @param bIsHover - [추가] 마우스가 현재 버튼 위에 있는지 여부
 */
void StartButton_Paint(Gdiplus::Graphics* gfx, RECT taskbarRect, Gdiplus::Image* imgStart, BOOL bIsHover);