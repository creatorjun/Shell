// Desktop.cpp : ����ȭ�� ���� ����� �����մϴ�.
//

// windows.h �� GDI+�� min/max ��ũ�� �浹�� �����մϴ�.
#define NOMINMAX

#include "framework.h"
#include "Desktop.h" // �� ����� ���
#include "Resource.h"
#include <stdio.h> // [�߰�] swprintf_s �Լ��� ����ϱ� ���� �߰�

// --- [GDI+ ����] ---
#include <Objidl.h>          // IStream ���� ���� GDI+���� ���� ����
#include <gdiplus.h>         // GDI+ ���
using namespace Gdiplus;     // GDI+ ���ӽ����̽�
// --- [GDI+ ��] ---


/**
 * @brief GDI+ �̹��� ���ڴ� CLSID�� ã�� ���� �Լ� (���� �м� ��� ������)
 * @param format (��: L"image/jpeg")
 * @param pClsid (���) ã�� CLSID
 * @return ���� �� �ε���(0 �̻�), ���� �� -1
 */
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT  num = 0;          // number of image encoders
    UINT  size = 0;         // size of the image encoder array in bytes

    ImageCodecInfo* pImageCodecInfo = nullptr; // C++ ��Ÿ�� ������

    GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1;  // Failure

    // [����] malloc ��� new[] �� ����Ͽ� C++ ��Ÿ�Ϸ� �޸� �Ҵ�
    pImageCodecInfo = (ImageCodecInfo*)(new BYTE[size]);
    if (pImageCodecInfo == nullptr)
        return -1;  // Failure

    // [����] GetImageEncoders�� Status�� Ȯ���Ͽ� ���� �� ��� ��ȯ (��� �ذ�)
    if (GetImageEncoders(num, size, pImageCodecInfo) != Ok)
    {
        delete[](BYTE*)pImageCodecInfo; // new BYTE[]�� �Ҵ������Ƿ� delete[] (BYTE*)�� ĳ���� ����
        return -1;
    }

    int result = -1; // ��� ���� ����

    for (UINT j = 0; j < num; ++j)
    {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            result = j; // ã������, break���� �ʰ� �޸� �������� �̵�
            break;
        }
    }

    // [����] delete[] �� ����Ͽ� �޸� ����
    delete[](BYTE*)pImageCodecInfo;
    return result;  // ã�Ұų�(-1�� �ƴ�) �� ã��(-1) ��� ��ȯ
}


/**
 * @brief GDI+ Image ��ü�� �ӽ� ���Ϸ� �����ϰ� ����ȭ������ �����մϴ�.
 * @param imgBackground (�Է�) ���ҽ����� �ε�� ��� GDI+ Image ��ü
 */
void Desktop_SetWallpaper(Gdiplus::Image* imgBackground)
{
    if (!imgBackground)
    {
        return;
    }

    CLSID jpgClsid;
    WCHAR tempPath[MAX_PATH];
    WCHAR jpgTempFilePath[MAX_PATH];

    // 1. JPEG ���ڴ� CLSID ��������
    if (GetEncoderClsid(L"image/jpeg", &jpgClsid) != -1)
    {
        // 2. Windows �ӽ� ���� ��� ��������
        if (GetTempPathW(MAX_PATH, tempPath))
        {
            // 3. �ӽ� ���� ��� ���� (��: C:\...Data\Local\Temp\MyShellBG.jpg)
            swprintf_s(jpgTempFilePath, MAX_PATH, L"%sMyShellBG.jpg", tempPath);

            // 4. GDI+ Image ��ü�� ���� ���Ϸ� ����
            if (imgBackground->Save(jpgTempFilePath, &jpgClsid, NULL) == Ok)
            {
                // 5. Windows API�� ȣ���Ͽ� ����ȭ�� ���� �� �ý��ۿ� �˸�
                SystemParametersInfoW(
                    SPI_SETDESKWALLPAPER,
                    0,
                    (PVOID)jpgTempFilePath,
                    SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE
                );
            }
        }
    }
}