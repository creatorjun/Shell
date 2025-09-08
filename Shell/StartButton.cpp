// creatorjun/shell/Shell-04869ccf080a38ea887a3d00139afecc32776daa/Shell/StartButton.cpp

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


// --- [수정: 링크 오류 해결] ---
/**
 * @brief GraphicsPath 객체에 둥근 사각형 경로를 추가하는 헬퍼 함수입니다.
 * [수정] 'static'을 추가하여 이 함수가 이 파일(.cpp) 내부에서만 보이도록 합니다.
 * (링커 충돌 방지)
 * @param path (입출력) 경로를 추가할 GDI+ GraphicsPath 객체 포인터
 * @param rect (입력) 사각형의 위치와 크기 (RectF 사용)
 * @param cornerRadius (입력) 모서리의 반경
 */
static void CreateRoundedRectPath(GraphicsPath* path, RectF rect, REAL cornerRadius)
{
    if (!path) return;

    REAL dia = cornerRadius * 2.0f;
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
// --- [수정 끝] ---


/**
 * @brief 작업표시줄 HDC(Graphics 객체) 위에 시작 버튼 이미지를 그립니다.
 * @param gfx - 그리기 대상인 작업표시줄의 GDI+ Graphics 객체 포인터
 * @param taskbarRect - 작업표시줄 창의 전체 RECT (좌표 계산에 사용됨)
 * @param imgStart - 리소스에서 로드된 시작 버튼 GDI+ Image 객체 포인터
 * @param bIsHover - [수정] 마우스 호버 상태 플래그
 */
void StartButton_Paint(Gdiplus::Graphics* gfx, RECT taskbarRect, Gdiplus::Image* imgStart, BOOL bIsHover)
{
    // Graphics 객체나 이미지 포인터가 유효하지 않으면 아무것도 하지 않음
    if (!gfx || !imgStart)
    {
        return;
    }

    // 작업표시줄의 전체 너비와 높이 계산
    int clientWidth = taskbarRect.right - taskbarRect.left;
    int clientHeight = taskbarRect.bottom - taskbarRect.top;

    // --- [수정: 호버 및 아이콘 영역 정의] ---

    // 1. 아이콘 크기 (기존)
    const int targetSize = 32;
    int posX = (clientWidth - targetSize) / 2;
    int posY = (clientHeight - targetSize) / 2;

    // 2. 호버 영역 크기 (아이콘보다 약간 큰 40x40)
    const int hoverSize = 40;
    int hoverX = (clientWidth - hoverSize) / 2;
    int hoverY = (clientHeight - hoverSize) / 2;

    // --- [새 코드: 호버 상태 그리기 (사용자 정의 색상/반경 기억)] ---
    if (bIsHover)
    {
        // 30% 불투명 하늘색 (Alpha: 76, R: 204, G: 153, B: 255)
        SolidBrush hoverBrush(Color(76, 204, 153, 255));
        REAL cornerRadius = 0.0f; // 직각 사각형

        // GDI+용 RectF 정의
        RectF hoverRectF((REAL)hoverX, (REAL)hoverY, (REAL)hoverSize, (REAL)hoverSize);

        // 둥근 사각형 경로 생성
        GraphicsPath path;
        CreateRoundedRectPath(&path, hoverRectF, cornerRadius);

        // 아이콘을 그리기 *전에* 경로를 채워서 배경 음영 효과를 줍니다.
        gfx->FillPath(&hoverBrush, &path);
    }
    // --- [코드 끝] ---


    // --- [최적화 1: 이미지 품질 설정] ---
    InterpolationMode oldMode = gfx->GetInterpolationMode(); // 기존 모드 백업
    gfx->SetInterpolationMode(InterpolationModeHighQualityBicubic);
    // --- ---


    // GDI+를 사용하여 이미지 그리기 (리사이징 포함)
    gfx->DrawImage(imgStart, posX, posY, targetSize, targetSize);


    // --- [최적화: 그래픽스 상태 복원] ---
    gfx->SetInterpolationMode(oldMode); // 보간 모드를 원래대로 복원
    // --- [최적화 끝] ---
}