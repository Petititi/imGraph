
#include <windows.h>
#include <dshow.h>
//#include <iostream>
#include <memory>
#include "ICVCam.h"

#pragma comment(lib, "strmiids")

using namespace std;

EXTERN_C const GUID FAR CLSID_VirtualCam = { 0x536b4138, 0xa300, 0x4355, { 0xb1, 0x89, 0x31, 0x18, 0x73, 0xe, 0xe7, 0x2 } };
EXTERN_C const GUID FAR IID_ICVCAM = { 0x1b0a7d7a, 0x776d, 0x46a8, { 0xb1, 0x80, 0x50, 0x7e, 0x61, 0xb3, 0x39, 0xce } };

HRESULT EnumerateDevices(REFGUID category, IEnumMoniker **ppEnum)
{
    // Create the System Device Enumerator.
    ICreateDevEnum *pDevEnum;
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
        CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));

    if (SUCCEEDED(hr))
    {
        // Create an enumerator for the category.
        hr = pDevEnum->CreateClassEnumerator(category, ppEnum, 0);
        if (hr == S_FALSE)
        {
            hr = VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
        }
        pDevEnum->Release();
    }
    return hr;
}

HRESULT GetVirtualCamInterface(IEnumMoniker* pEnum, ICVCam** ppICVCam)
{
    HRESULT r_hr = E_FAIL;
    IMoniker *pMoniker;
    IBaseFilter *filter;
    ICVCam* pICVCam;

    while (pEnum->Next(1, &pMoniker, NULL) == S_OK)
    {
        HRESULT hr = pMoniker->BindToObject(0, 0, IID_PPV_ARGS(&filter));
        if (SUCCEEDED(hr))
        {
            CLSID vcam;
            hr = filter->GetClassID(&vcam);
            if (SUCCEEDED(hr)) {
                if (IsEqualCLSID(vcam, CLSID_VirtualCam)) {
                    hr = filter->QueryInterface(IID_ICVCAM, (void**)(&pICVCam));
                    if (SUCCEEDED(hr)) {
                        *ppICVCam = pICVCam;
                        r_hr = S_OK;
                    }
                }
            }
            filter->Release();
        }
        pMoniker->Release();
    }
    return r_hr;
}

void main()
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    IEnumMoniker* pEnumMoniker;
    ICVCam* pVCam;
    LONG width, height;

    IRunningObjectTable *pROT;
    IMoniker *pMoniker;
    IBindCtx * pBindCtx;
    IBaseFilter *pFilter;
    OLECHAR* moniker_name;
    CLSID vcam;
    IPropertyBag *pPropBag;
    IUnknown *pUnknown;

    if (SUCCEEDED(hr)) {
        hr = GetRunningObjectTable(0, &pROT);
        if (SUCCEEDED(hr)) {
            pROT->EnumRunning(&pEnumMoniker);
            if (SUCCEEDED(hr)) {
                while (pEnumMoniker->Next(1, &pMoniker, NULL) == S_OK)
                {
                    hr = CreateBindCtx(0, &pBindCtx);
                    if (SUCCEEDED(hr)) {
                        pMoniker->GetDisplayName(pBindCtx, NULL, &moniker_name);
                        wprintf(L"DisplayName is %s\n", moniker_name);
                        
                        hr = pROT->GetObject(pMoniker, (IUnknown**)&pFilter);

                        if (SUCCEEDED(hr)) {
                            pFilter->Release();
                        }

                        //hr = pMoniker->BindToStorage(pBindCtx, 0, IID_IPropertyBag, (void**)&pPropBag);
                        //if (SUCCEEDED(hr)) {
                        //    pPropBag->Release();
                        //}
                        CoTaskMemFree(moniker_name);
                        pBindCtx->Release();
                    }

                    pMoniker->Release();
                }
                pEnumMoniker->Release();
            }
            pROT->Release();
        }
    }



    //if (SUCCEEDED(hr))
    //{
    //    hr = EnumerateDevices(CLSID_VideoInputDeviceCategory, &pEnumMoniker);
    //    if (SUCCEEDED(hr))
    //    {
    //        //DisplayDeviceInformation(pEnum);
    //        hr = GetVirtualCamInterface(pEnumMoniker, &pVCam);
    //        if (SUCCEEDED(hr)) {
    //            while (SUCCEEDED(hr)) {
    //                hr = pVCam->GetSize(&width, &height);
    //                printf("%dx%d\n", width, height);
    //                Sleep(1000);
    //            }
    //            pVCam->Release();
    //        } else {
    //            printf("Couldn't find any virtual camera\n");
    //        }
    //        pEnumMoniker->Release();
    //    }
    //    CoUninitialize();
    //}
}