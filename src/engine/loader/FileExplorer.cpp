/**
 * @file FileExplorer.cpp
 * @author Ryan Purse
 * @date 07/10/2023
 */


#include "FileExplorer.h"
#include <Windows.h>
#include <shobjidl.h>
#include <Statistics.h>


std::string openFileExplorer()
{
    HRESULT fSysHr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(fSysHr))
    {
        WARN("Failed to initialise file object system.");
        return "";
    }
    
    IFileOpenDialog *fFileSystem;
    fSysHr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&fFileSystem));
    if (FAILED(fSysHr))
    {
        WARN("Failed to create file open dialog.");
        CoUninitialize();
        return "";
    }
    
    fSysHr = fFileSystem->Show(nullptr);
    if (FAILED(fSysHr))
    {
        WARN("Failed to show file explorer.");
        fFileSystem->Release();
        CoUninitialize();
        return "";
    }
    
    IShellItem *fFiles;
    fSysHr = fFileSystem->GetResult(&fFiles);
    if (FAILED(fSysHr))
    {
        WARN("Failed to get value from file explorer.");
        fFileSystem->Release();
        CoUninitialize();
        return "";
    }
    
    PWSTR fPath;
    fSysHr = fFiles->GetDisplayName(SIGDN_FILESYSPATH, &fPath);
    if (FAILED(fSysHr))
    {
        WARN("Failed to get the file path for the item.");
        fFiles->Release();
        fFileSystem->Release();
        CoUninitialize();
        return "";
    }
    
    std::wstring path(fPath);
    std::vector<char> buf(path.size());  // We're doing this to avoid the possible loss of data message.
    std::transform(path.begin(), path.end(), buf.begin(), [](wchar_t c) { return static_cast<char>(c); });
    std::string c(buf.begin(), buf.end());
    
    CoTaskMemFree(fPath);
    fFiles->Release();
    fFileSystem->Release();
    CoUninitialize();
    
    return c;
}
