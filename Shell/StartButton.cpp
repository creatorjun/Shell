// StartButton.cpp : 시작 버튼 그리기 모듈을 구현합니다.
//

// windows.h 와 GDI+의 min/max 매크로 충돌을 방지합니다.
#define NOMINMAX

#include "StartButton.h" // 이 모듈의 헤더

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
 */
void StartButton_Paint(Gdiplus::Graphics* gfx, RECT taskbarRect, Gdiplus::Image* imgStart)
{
    // Graphics 객체나 이미지 포인터가 유효하지 않으면 아무것도 하지 않음
    if (!gfx || !imgStart)
    {
        return;
    }

    // 작업표시줄의 전체 너비와 높이 계산
    int clientWidth = taskbarRect.right - taskbarRect.left;
    int clientHeight = taskbarRect.bottom - taskbarRect.top;

    // 시작 버튼을 32x32 크기(고정)로 설정
    const int targetSize = 32;

    // 가운데 좌표 계산: (전체 너비 / 2) - (이미지 너비 / 2)
    int posX = (clientWidth - targetSize) / 2;
    // 세로 중앙 좌표 계산: (전체 높이 / 2) - (이미지 높이 / 2)
    int posY = (clientHeight - targetSize) / 2;


    // --- [최적화 1: 이미지 품질 설정 (오류 수정됨)] ---
    // (수정) GetInterpolationMode는 값을 직접 반환합니다.
    InterpolationMode oldMode = gfx->GetInterpolationMode(); // 기존 모드 백업
    gfx->SetInterpolationMode(InterpolationModeHighQualityBicubic);
    // --- [수정 끝] ---


    // GDI+를 사용하여 이미지 그리기 (리사이징 포함)
    gfx->DrawImage(imgStart, posX, posY, targetSize, targetSize);


    // --- [최적화: 그래픽스 상태 복원] ---
    gfx->SetInterpolationMode(oldMode); // 보간 모드를 원래대로 복원
    // --- [최적화 끝] ---
}