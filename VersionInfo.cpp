#include <windows.h>
#include <VersionHelpers.h>
#include <string>
#include<tchar.h>

#include"VersionInfo.h"

// Windowsのバージョン情報を取得
    std::string GetOSVersion(){
        std::string version;

        if (IsWindowsXPOrGreater())
        {
            version += "XPOrGreater\n";
        }

        if (IsWindowsXPSP1OrGreater())
        {
            version += "XPSP1OrGreater\n";
        }

        if (IsWindowsXPSP2OrGreater())
        {
            version += "XPSP2OrGreater\n";
        }

        if (IsWindowsXPSP3OrGreater())
        {
            version += "XPSP3OrGreater\n";
        }

        if (IsWindowsVistaOrGreater())
        {
            version += "VistaOrGreater\n";
        }

        if (IsWindowsVistaSP1OrGreater())
        {
            version += "VistaSP1OrGreater\n";
        }

        if (IsWindowsVistaSP2OrGreater())
        {
            version += "VistaSP2OrGreater\n";
        }

        if (IsWindows7OrGreater())
        {
            version += "Windows7OrGreater\n";
        }

        if (IsWindows7SP1OrGreater())
        {
            version += "Windows7SP1OrGreater\n";
        }

        if (IsWindows8OrGreater())
        {
            version += "Windows8OrGreater\n";
        }

        if (IsWindows8Point1OrGreater())
        {
            version += "Windows8Point1OrGreater\n";
        }

        if (IsWindows10OrGreater())
        {
            version += "Windows10OrGreater\n";
        }

        if (IsWindowsServer())
        {
            version += "Server\n";
        }
        else
        {
            version += "Client\n";
        }

        return version;
    }

    // メッセージボックスに出力
    bool SendVersionInfoToMessageBox(HWND hWnd) {
        std::string osVersion = GetOSVersion();
        MessageBox(hWnd, osVersion.c_str(), _T("Windows OS Version"), MB_OK);
        return true;
    }