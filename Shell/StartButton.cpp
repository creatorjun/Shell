// creatorjun/shell/Shell-04869ccf080a38ea887a3d00139afecc32776daa/Shell/StartButton.cpp

// StartButton.cpp : 시작 버튼 그리기 모듈을 구현합니다.
//

// windows.h 와 GDI+의 min/max 매크로 충돌을 방지합니다.
#define NOMINMAX

#include "framework.h"
#include "StartButton.h"    // 이 모듈의 헤더
#include "GraphicsUtil.h"   // 둥근 사각형 헬퍼 함수를 위해 포함
#include "Constants.h"      // [추가] 상수 정의 헤더를 포함합니다.

// --- GDI+ 설정 ---
#include <Objidl.h>          // IStream 등을 위해 GDI+보다 먼저 포함
#include <gdiplus.h>         // GDI+ 헤더
using namespace Gdiplus;     // GDI+ 네임스페이스
// --- GDI+ 끝 ---


/**
 * @brief 작업표시줄 HDC(Graphics 객체) 위에 시작 버튼 이미지를 그립니다.
 * @param gfx - 그리기 대상인 작업표시줄의 GDI+ Graphics 객체 포인터
 * @param taskbarRect - 작업표시줄 창의 전체 RECT (좌표 계산에 사용됨)
 * @param imgStart - 리소스에서 로드된 시작 버튼 GDI+ Image 객체 포인터
 * @param bIsHover - 마우스 호버 상태 플래그
 */
void StartButton_Paint(Gdiplus::Graphics* gfx, RECT taskbarRect, Gdiplus::Image* imgStart, BOOL bIsHover)
{
    if (!gfx || !imgStart)
    {
        return;
    }

    int clientWidth = taskbarRect.right - taskbarRect.left;
    int clientHeight = taskbarRect.bottom - taskbarRect.top;

    // [수정] 상수 사용
    // 1. 아이콘 크기
    int posX = (clientWidth - START_BUTTON_SIZE) / 2;
    int posY = (clientHeight - START_BUTTON_SIZE) / 2;

    // 2. 호버 영역 크기
    int hoverX = (clientWidth - START_BUTTON_HOVER_SIZE) / 2;
    int hoverY = (clientHeight - START_BUTTON_HOVER_SIZE) / 2;

    if (bIsHover)
    {
        SolidBrush hoverBrush(Color(76, 204, 153, 255));
        float cornerRadius = 0.0f;

        // [수정] 상수 사용
        RectF hoverRectF((REAL)hoverX, (REAL)hoverY, (REAL)START_BUTTON_HOVER_SIZE, (REAL)START_BUTTON_HOVER_SIZE);

        GraphicsPath path;
        CreateRoundedRectPath(&path, hoverRectF, cornerRadius);
        gfx->FillPath(&hoverBrush, &path);
    }

    InterpolationMode oldMode = gfx->GetInterpolationMode();
    gfx->SetInterpolationMode(InterpolationModeHighQualityBicubic);

    // [수정] 상수 사용
    gfx->DrawImage(imgStart, posX, posY, START_BUTTON_SIZE, START_BUTTON_SIZE);

    gfx->SetInterpolationMode(oldMode);
}