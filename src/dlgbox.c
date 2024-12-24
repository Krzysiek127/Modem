#include "dlgbox.h"

wchar_t* OpenFileDialog(void)
{
    HRESULT hr;
    IFileDialog* pFileDialog = NULL;
    wchar_t* pszFilePath = NULL;

    // Initialize COM library
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr))
        return NULL; // COM initialization failed

    // Create the File Open Dialog object
    hr = CoCreateInstance(&CLSID_FileOpenDialog, NULL, CLSCTX_ALL, &IID_IFileDialog, (void**)&pFileDialog);
    if (SUCCEEDED(hr))
    {
        // Set the file types to *.*
        COMDLG_FILTERSPEC fileTypes[] = {
            { L"All Files", L"*.*" }
        };

        // Set file dialog options
        DWORD dwFlags;
        hr = pFileDialog->lpVtbl->GetOptions(pFileDialog, &dwFlags);
        if (SUCCEEDED(hr))
            hr = pFileDialog->lpVtbl->SetOptions(pFileDialog, dwFlags | FOS_FORCEFILESYSTEM);

        // Set the file type filter
        pFileDialog->lpVtbl->SetFileTypes(pFileDialog, ARRAYSIZE(fileTypes), fileTypes);

        // Show the dialog
        hr = pFileDialog->lpVtbl->Show(pFileDialog, NULL);
        if (SUCCEEDED(hr))
        {
            // Get the result
            IShellItem* pItem = NULL;
            hr = pFileDialog->lpVtbl->GetResult(pFileDialog, &pItem);
            if (SUCCEEDED(hr))
            {
                wchar_t *tempFilePath = NULL;
                hr = pItem->lpVtbl->GetDisplayName(pItem, SIGDN_FILESYSPATH, &tempFilePath);
                if (SUCCEEDED(hr))
                {
                    // Allocate memory for the file path and copy the result
                    size_t len = wcslen(tempFilePath) + 1;
                    pszFilePath = (wchar_t*)malloc(len * sizeof(wchar_t));

                    if (pszFilePath != NULL)
                        wcscpy_s(pszFilePath, len, tempFilePath);

                    CoTaskMemFree(tempFilePath);
                }
                pItem->lpVtbl->Release(pItem);
            }
        }

        // Release the file dialog object
        pFileDialog->lpVtbl->Release(pFileDialog);
    }

    // Uninitialize COM
    CoUninitialize();

    return pszFilePath; // Returns the file path or NULL if failed
}