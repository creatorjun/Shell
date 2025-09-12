// Desktop.cpp : 바탕화면 관련 기능을 구현합니다.
//

// windows.h 와 GDI+의 min/max 매크로 충돌을 방지합니다.
#define NOMINMAX

#include "framework.h"
#include "Desktop.h" // 이 모듈의 헤더
#include "Resource.h"
#include <stdio.h> // [추가] swprintf_s 함수를 사용하기 위해 추가

// --- [GDI+ 설정] ---
#include <Objidl.h>          // IStream 등을 위해 GDI+보다 먼저 포함
#include <gdiplus.h>         // GDI+ 헤더
using namespace Gdiplus;     // GDI+ 네임스페이스
// --- [GDI+ 끝] ---


/**
 * @brief GDI+ 이미지 인코더 CLSID를 찾는 헬퍼 함수 (정적 분석 경고 수정됨)
 * @param format (예: L"image/jpeg")
 * @param pClsid (출력) 찾은 CLSID
 * @return 성공 시 인덱스(0 이상), 실패 시 -1
 */
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT  num = 0;          // number of image encoders
    UINT  size = 0;         // size of the image encoder array in bytes

    ImageCodecInfo* pImageCodecInfo = nullptr; // C++ 스타일 포인터

    GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1;  // Failure

    // [수정] malloc 대신 new[] 를 사용하여 C++ 스타일로 메모리 할당
    pImageCodecInfo = (ImageCodecInfo*)(new BYTE[size]);
    if (pImageCodecInfo == nullptr)
        return -1;  // Failure

    // [수정] GetImageEncoders의 Status를 확인하여 실패 시 즉시 반환 (경고 해결)
    if (GetImageEncoders(num, size, pImageCodecInfo) != Ok)
    {
        delete[](BYTE*)pImageCodecInfo; // new BYTE[]로 할당했으므로 delete[] (BYTE*)로 캐스팅 해제
        return -1;
    }

    int result = -1; // 결과 저장 변수

    for (UINT j = 0; j < num; ++j)
    {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            result = j; // 찾았지만, break하지 않고 메모리 해제까지 이동
            break;
        }
    }

    // [수정] delete[] 를 사용하여 메모리 해제
    delete[](BYTE*)pImageCodecInfo;
    return result;  // 찾았거나(-1이 아님) 못 찾은(-1) 결과 반환
}


/**
 * @brief GDI+ Image 객체를 임시 파일로 저장하고 바탕화면으로 설정합니다.
 * @param imgBackground (입력) 리소스에서 로드된 배경 GDI+ Image 객체
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

    // 1. JPEG 인코더 CLSID 가져오기
    if (GetEncoderClsid(L"image/jpeg", &jpgClsid) != -1)
    {
        // 2. Windows 임시 폴더 경로 가져오기
        if (GetTempPathW(MAX_PATH, tempPath))
        {
            // 3. 임시 파일 경로 생성 (예: C:\...Data\Local\Temp\MyShellBG.jpg)
            swprintf_s(jpgTempFilePath, MAX_PATH, L"%sMyShellBG.jpg", tempPath);

            // 4. GDI+ Image 객체를 실제 파일로 저장
            if (imgBackground->Save(jpgTempFilePath, &jpgClsid, NULL) == Ok)
            {
                // 5. Windows API를 호출하여 바탕화면 변경 및 시스템에 알림
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