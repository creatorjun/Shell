#pragma once

// GDI+의 Image 클래스를 전방 선언합니다.
// (헤더 파일에 GDI+ 전체를 포함하지 않기 위함)
namespace Gdiplus
{
    class Image;
}

/**
 * @brief 바탕화면 배경화면을 설정합니다.
 * @param imgBackground - 리소스에서 로드된 배경화면 GDI+ Image 객체 포인터
 */
void Desktop_SetWallpaper(Gdiplus::Image* imgBackground);