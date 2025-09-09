#pragma once
#include <windows.h> // ATOM, BOOL, HINSTANCE �� Windows Ÿ���� ����ϱ� ���� ����
#include <memory>    // std::unique_ptr ����� ���� �߰�

// GDI+ Ŭ���� ���� ����
namespace Gdiplus
{
    class Image;
}

/**
 * @brief �۾�ǥ����(AppBar)�� ������ Ŭ������ �ý��ۿ� ����մϴ�.
 * @param hInstance - ���ø����̼� �ν��Ͻ�
 * @param szWindowClass - ����� â Ŭ���� �̸�
 * @return ATOM ������ Ŭ���� Atom
 */
ATOM Taskbar_Register(HINSTANCE hInstance, const WCHAR* szWindowClass);

/**
 * @brief �۾�ǥ����(AppBar) �����츦 �����ϰ� ȭ�鿡 ǥ���մϴ�.
 * @param hInst - ���ø����̼� �ν��Ͻ�
 * @param nCmdShow - ������ ǥ�� ����
 * @param szTitle - â Ÿ��Ʋ
 * @param szWindowClass - ��ϵ� â Ŭ���� �̸�
 * @param bg - [����] �۾�ǥ���� ������� ����� GDI+ Image ��ü�� �������� ���� std::unique_ptr
 * @param start - [����] ���� ��ư���� ����� GDI+ Image ��ü�� �������� ���� std::unique_ptr
 * @return ���� �� TRUE, ���� �� FALSE
 */
BOOL Taskbar_Create(HINSTANCE hInst, int nCmdShow,
    const WCHAR* szTitle, const WCHAR* szWindowClass,
    std::unique_ptr<Gdiplus::Image> bg, std::unique_ptr<Gdiplus::Image> start);