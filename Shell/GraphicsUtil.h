#pragma once

// �ʿ��� GDI+ Ŭ������ ���� �����մϴ�.
// �̷��� �ϸ� ��� ���Ͽ� GDI+ ��ü�� ������ �ʿ䰡 ��������,
// �����Ϸ��� Ÿ���� �ν��� �� �ְ� �˴ϴ�.
namespace Gdiplus
{
    class GraphicsPath;
    struct RectF; // RectF�� ����ü�̹Ƿ� ���� ������ �����մϴ�.
}

/**
 * @brief GraphicsPath ��ü�� �ձ� �簢�� ��θ� �߰��ϴ� ���� �Լ��Դϴ�.
 * @param path (�����) ��θ� �߰��� GDI+ GraphicsPath ��ü ������
 * @param rect (�Է�) �簢���� ��ġ�� ũ�� (RectF ���)
 * @param cornerRadius (�Է�) �𼭸��� �ݰ�
 */
void CreateRoundedRectPath(Gdiplus::GraphicsPath* path, Gdiplus::RectF rect, float cornerRadius);