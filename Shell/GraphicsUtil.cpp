// [�߰�] windows.h �� GDI+�� min/max ��ũ�� �浹�� �����մϴ�.
#define NOMINMAX

#include "framework.h"
#include "GraphicsUtil.h"

// [�߰�] GDI+�� �����ϴ� ������� �����մϴ�.
#include <objidl.h>

// [�߰�] GDI+�� ��ü ���Ǹ� ����ϱ� ���� cpp ���Ͽ��� ���� �����մϴ�.
#include <gdiplus.h>

/**
 * @brief GraphicsPath ��ü�� �ձ� �簢�� ��θ� �߰��ϴ� ���� �Լ��Դϴ�.
 * @param path (�����) ��θ� �߰��� GDI+ GraphicsPath ��ü ������
 * @param rect (�Է�) �簢���� ��ġ�� ũ�� (RectF ���)
 * @param cornerRadius (�Է�) �𼭸��� �ݰ�
 */
void CreateRoundedRectPath(Gdiplus::GraphicsPath* path, Gdiplus::RectF rect, float cornerRadius)
{
    if (!path) return;

    // �𼭸� �ݰ��� 2�� (����)
    float dia = cornerRadius * 2.0f;
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