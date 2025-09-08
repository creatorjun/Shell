// creatorjun/shell/Shell-04869ccf080a38ea887a3d00139afecc32776daa/Shell/StartButton.cpp

// StartButton.cpp : ���� ��ư �׸��� ����� �����մϴ�.
//

// windows.h �� GDI+�� min/max ��ũ�� �浹�� �����մϴ�.
#define NOMINMAX

#include "StartButton.h" // �� ����� ���

// --- GDI+ ���� ---
#include <Objidl.h>          // IStream ���� ���� GDI+���� ���� ����
#include <gdiplus.h>         // GDI+ ���
using namespace Gdiplus;     // GDI+ ���ӽ����̽�
// --- GDI+ �� ---


// --- [����: ��ũ ���� �ذ�] ---
/**
 * @brief GraphicsPath ��ü�� �ձ� �簢�� ��θ� �߰��ϴ� ���� �Լ��Դϴ�.
 * [����] 'static'�� �߰��Ͽ� �� �Լ��� �� ����(.cpp) ���ο����� ���̵��� �մϴ�.
 * (��Ŀ �浹 ����)
 * @param path (�����) ��θ� �߰��� GDI+ GraphicsPath ��ü ������
 * @param rect (�Է�) �簢���� ��ġ�� ũ�� (RectF ���)
 * @param cornerRadius (�Է�) �𼭸��� �ݰ�
 */
static void CreateRoundedRectPath(GraphicsPath* path, RectF rect, REAL cornerRadius)
{
    if (!path) return;

    REAL dia = cornerRadius * 2.0f;
    if (dia <= 0.0f)
    {
        // �ݰ��� 0�̸� �ܼ� �簢�� �߰�
        path->AddRectangle(rect);
        return;
    }

    // GDI+�� 4���� ȣ�� 4���� ���� ���� �����ؾ� �ձ� �簢���� �˴ϴ�.
    // (�ð� ����: �»�� -> ���� -> ���ϴ� -> ���ϴ�)
    path->AddArc(rect.X, rect.Y, dia, dia, 180, 90); // �»�� ȣ
    path->AddArc(rect.GetRight() - dia, rect.Y, dia, dia, 270, 90); // ���� ȣ
    path->AddArc(rect.GetRight() - dia, rect.GetBottom() - dia, dia, dia, 0, 90); // ���ϴ� ȣ
    path->AddArc(rect.X, rect.GetBottom() - dia, dia, dia, 90, 90); // ���ϴ� ȣ

    // ��θ� �ݾ� ������ �ϼ��մϴ�.
    path->CloseFigure();
}
// --- [���� ��] ---


/**
 * @brief �۾�ǥ���� HDC(Graphics ��ü) ���� ���� ��ư �̹����� �׸��ϴ�.
 * @param gfx - �׸��� ����� �۾�ǥ������ GDI+ Graphics ��ü ������
 * @param taskbarRect - �۾�ǥ���� â�� ��ü RECT (��ǥ ��꿡 ����)
 * @param imgStart - ���ҽ����� �ε�� ���� ��ư GDI+ Image ��ü ������
 * @param bIsHover - [����] ���콺 ȣ�� ���� �÷���
 */
void StartButton_Paint(Gdiplus::Graphics* gfx, RECT taskbarRect, Gdiplus::Image* imgStart, BOOL bIsHover)
{
    // Graphics ��ü�� �̹��� �����Ͱ� ��ȿ���� ������ �ƹ��͵� ���� ����
    if (!gfx || !imgStart)
    {
        return;
    }

    // �۾�ǥ������ ��ü �ʺ�� ���� ���
    int clientWidth = taskbarRect.right - taskbarRect.left;
    int clientHeight = taskbarRect.bottom - taskbarRect.top;

    // --- [����: ȣ�� �� ������ ���� ����] ---

    // 1. ������ ũ�� (����)
    const int targetSize = 32;
    int posX = (clientWidth - targetSize) / 2;
    int posY = (clientHeight - targetSize) / 2;

    // 2. ȣ�� ���� ũ�� (�����ܺ��� �ణ ū 40x40)
    const int hoverSize = 40;
    int hoverX = (clientWidth - hoverSize) / 2;
    int hoverY = (clientHeight - hoverSize) / 2;

    // --- [�� �ڵ�: ȣ�� ���� �׸��� (����� ���� ����/�ݰ� ���)] ---
    if (bIsHover)
    {
        // 30% ������ �ϴû� (Alpha: 76, R: 204, G: 153, B: 255)
        SolidBrush hoverBrush(Color(76, 204, 153, 255));
        REAL cornerRadius = 0.0f; // ���� �簢��

        // GDI+�� RectF ����
        RectF hoverRectF((REAL)hoverX, (REAL)hoverY, (REAL)hoverSize, (REAL)hoverSize);

        // �ձ� �簢�� ��� ����
        GraphicsPath path;
        CreateRoundedRectPath(&path, hoverRectF, cornerRadius);

        // �������� �׸��� *����* ��θ� ä���� ��� ���� ȿ���� �ݴϴ�.
        gfx->FillPath(&hoverBrush, &path);
    }
    // --- [�ڵ� ��] ---


    // --- [����ȭ 1: �̹��� ǰ�� ����] ---
    InterpolationMode oldMode = gfx->GetInterpolationMode(); // ���� ��� ���
    gfx->SetInterpolationMode(InterpolationModeHighQualityBicubic);
    // --- ---


    // GDI+�� ����Ͽ� �̹��� �׸��� (������¡ ����)
    gfx->DrawImage(imgStart, posX, posY, targetSize, targetSize);


    // --- [����ȭ: �׷��Ƚ� ���� ����] ---
    gfx->SetInterpolationMode(oldMode); // ���� ��带 ������� ����
    // --- [����ȭ ��] ---
}