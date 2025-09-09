#pragma once

// �ʿ��� GDI+ Ŭ������ ���� �����մϴ�.
namespace Gdiplus
{
    class GraphicsPath;
    // [����] RectF�� struct ��� class�� ���� �����Ͽ� ��� C4099�� �ذ��մϴ�.
    class RectF;
}

/**
 * @brief GraphicsPath ��ü�� �ձ� �簢�� ��θ� �߰��ϴ� ���� �Լ��Դϴ�.
 * @param path (�����) ��θ� �߰��� GDI+ GraphicsPath ��ü ������
 * @param rect (�Է�) �簢���� ��ġ�� ũ�� (RectF ���)
 * @param cornerRadius (�Է�) �𼭸��� �ݰ�
 */
void CreateRoundedRectPath(Gdiplus::GraphicsPath* path, Gdiplus::RectF rect, float cornerRadius);