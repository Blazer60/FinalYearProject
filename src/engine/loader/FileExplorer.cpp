/**
 * @file FileExplorer.cpp
 * @author Ryan Purse
 * @date 07/10/2023
 */


#include "FileExplorer.h"
#include "FileLoader.h"

#ifdef _WIN32
    #include <Windows.h>
    #include <shobjidl.h>
    #include <Statistics.h>
    #include <ShlGuid.h>
#endif  // WIN32



std::string openFileDialog()
{
#ifdef _WIN32
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
#else
    WARN("This function only works for windows.");
    return "";
#endif  // _WIN32
}

std::string saveFileDialog()
{
#ifdef _WIN32
    HRESULT fSysHr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(fSysHr))
    {
        WARN("Failed to initialise file object system.");
        return "";
    }
    
    IFileSaveDialog *fFileSystem;
    fSysHr = CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_ALL, IID_IFileSaveDialog, reinterpret_cast<void**>(&fFileSystem));
    if (FAILED(fSysHr))
    {
        WARN("Failed to create file save dialog.");
        CoUninitialize();
        return "";
    }
    
    COMDLG_FILTERSPEC types[2];
    types[0].pszName = L"Scene File";
    types[0].pszSpec = L"*.pcy";
    types[1].pszName = L"All Files";
    types[1].pszSpec = L"*.*";
    fFileSystem->SetFileTypes(2, types);
    fFileSystem->SetDefaultExtension(L".pcy");
    
    fSysHr = fFileSystem->Show(nullptr);
    if (FAILED(fSysHr))
    {
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
#else
    WARN("This function only works for windows.");
    return "";
#endif  // _WIN32
}
