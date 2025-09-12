#pragma once
#pragma once
#include <windows.h>

/**
 * @brief ����ȭ�� ��Ŭ�� �޴� ����� ĸ��ȭ�ϴ� Ŭ�����Դϴ�.
 */
class DesktopMenu
{
public:
    /**
     * @brief ����ȭ�� �޴� â�� ������ Ŭ������ ����մϴ�.
     * @param hInstance - ���ø����̼� �ν��Ͻ� �ڵ�
     * @return ���� �� RegisterClassExW�� ��ȯ�ϴ� ATOM ��, ���� �� 0
     */
    static ATOM Register(HINSTANCE hInstance);

    /**
     * @brief ����ȭ�� �޴� â�� �����մϴ�.
     * @param hInstance - ���ø����̼� �ν��Ͻ� �ڵ�
     * @param hOwner - �θ�(������) ������ �ڵ�
     * @return ���� �� ������ â �ڵ�(HWND), ���� �� NULL
     */
    static HWND Create(HINSTANCE hInstance, HWND hOwner);

    /**
     * @brief ������ ��ġ�� ����ȭ�� �޴� â�� ǥ���մϴ�.
     * @param hMenu - ǥ���� �޴� â�� �ڵ�
     * @param x - ȭ�� ���� ��� ���� x ��ǥ
     * @param y - ȭ�� ���� ��� ���� y ��ǥ
     */
    static void Show(HWND hMenu, int x, int y);

    /**
     * @brief ����ȭ�� �޴� â�� ����ϴ�.
     * @param hMenu - ���� �޴� â�� �ڵ�
     */
    static void Hide(HWND hMenu);
};