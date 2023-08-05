#include "explorer.h"
#include <windows.h>
#include <tchar.h>
#include <shlobj.h>
#include <shldisp.h>
#include <stdbool.h>
#include <stdio.h>

/* IMPORTANT: THIS CODE IS FROM https://stackoverflow.com/questions/43815932/how-to-get-the-path-of-an-active-file-explorer-window-in-c-winapi 
   I ASKED CHATGPT TO REFACTOR IT TO C WHILE MAKING IT PRINT THE PIDL INSTEAD OF PUSHING IT TO A LIST.
   IF THIS BREAKS, THEN IDK WHAT TO DO
*/
char* get_focused_explorer_path(){
    HWND tophwnd = GetForegroundWindow();
    if(tophwnd == NULL) return NULL;

    IShellWindows *pshWindows = NULL;
    HRESULT hr = CoCreateInstance(&CLSID_ShellWindows, NULL, CLSCTX_ALL, &IID_IShellWindows, (void**)&pshWindows);

    if (FAILED(hr)) {
        fprintf(stderr, "Could not create instance of IShellWindows\n");
        return NULL;
    }

    long count = 0;
    hr = pshWindows->lpVtbl->get_Count(pshWindows, &count);

    if (FAILED(hr)) {
        fprintf(stderr, "Could not get number of shell windows\n");
        return NULL;
    }

    for (long i = 0; i < count; ++i) {
        VARIANT vi;
        VariantInit(&vi);
        vi.vt = VT_I4;
        vi.lVal = i;
        IDispatch *pDisp = NULL;
        hr = pshWindows->lpVtbl->Item(pshWindows, vi, &pDisp);

        if (FAILED(hr) || !pDisp)
            continue;

        IWebBrowserApp *pApp = NULL;
        hr = pDisp->lpVtbl->QueryInterface(pDisp, &IID_IWebBrowserApp, (void**)&pApp);

        if (FAILED(hr) || !pApp)
            continue;

        HWND hwnd;
        pApp->lpVtbl->get_HWND(pApp, (SHANDLE_PTR*)&hwnd);
        if(hwnd != tophwnd)
            continue;

        IServiceProvider *psp = NULL;
        hr = pApp->lpVtbl->QueryInterface(pApp, &IID_IServiceProvider, (void**)&psp);

        if (FAILED(hr) || !psp)
            continue;

        IShellBrowser *pBrowser = NULL;
        hr = psp->lpVtbl->QueryService(psp, &SID_STopLevelBrowser, &IID_IShellBrowser, (void**)&pBrowser);

        if (FAILED(hr))
            continue;

        IShellView *pShellView = NULL;
        hr = pBrowser->lpVtbl->QueryActiveShellView(pBrowser, &pShellView);

        if (FAILED(hr))
            continue;

        IFolderView *pFolderView = NULL;
        hr = pShellView->lpVtbl->QueryInterface(pShellView, &IID_IFolderView, (void**)&pFolderView);

        if (FAILED(hr) || !pFolderView)
            continue;

        IPersistFolder2 *pFolder = NULL;
        hr = pFolderView->lpVtbl->GetFolder(pFolderView, &IID_IPersistFolder2, (void**)&pFolder);

        if (FAILED(hr))
            continue;

        LPITEMIDLIST pidl = NULL;
        hr = pFolder->lpVtbl->GetCurFolder(pFolder, &pidl);

        if (SUCCEEDED(hr))
        {
            TCHAR szPath[MAX_PATH];
            if (SHGetPathFromIDList(pidl, szPath))
            {
                CoTaskMemFree(pidl);
                return strdup(szPath);
            }
            else
            {
                printf("Failed to convert PIDL to path.\n");
            }
            // Free the PIDL when done.
            CoTaskMemFree(pidl);
        }
    }

    return NULL;
}

void add_explorer_path(int preset){
    char* path = get_focused_explorer_path();
    if(path == NULL)
        return;
    
    FILE* f = fopen("cfg/exp_presets", "r");
    if(f == NULL) {
        printf("WARNING: exp_presets missing");
    }

    FILE *temp_file = tmpfile();
    if (temp_file == NULL) {
        perror("Failed to create temporary file");
        fclose(f);
        return;
    }

    char buffer[MAX_PATH];
    int line = 0;
    while (fgets(buffer, sizeof(buffer), f)) {
        line++;
        if (line == preset) {
            fprintf(temp_file, "%s\n", path);
        } else {
            fprintf(temp_file, "%s", buffer);
        }
    }

    fclose(f);

    // Annoying...
    f = fopen("cfg/exp_presets", "w");
    if (f == NULL) {
        perror("Failed to open file for writing");
        fclose(temp_file);
        return;
    }

    rewind(temp_file);
    while (fgets(buffer, sizeof(buffer), temp_file)) {
        fprintf(f, "%s", buffer);
    }

    fclose(f);
    fclose(temp_file);

    free(path);
}

void open_explorer(int preset){
    FILE* f = fopen("cfg/exp_presets", "r");
    if(f == NULL) {
        printf("WARNING: exp_presets missing");
        return;
    }

    char path[256] = {0};
    int line = 0;
    while(fgets(path, sizeof(path), f)){
        line++;
        if(line == preset)
            break;
    }
    fclose(f);

    ShellExecute(NULL, "open", "explorer.exe", path, NULL, SW_SHOWDEFAULT);
}

void close_explorer(){
    HWND hwnd = GetForegroundWindow();

    char classname[256];
    GetClassName(hwnd, classname, sizeof(classname));

    if(strcmp(classname, "CabinetWClass") == 0)
        PostMessage(hwnd, WM_CLOSE, 0, 0);
}

bool open_vscode(){
    HWND hwnd = GetForegroundWindow();
    if(hwnd == NULL) return false;
    char command[256];
    snprintf(command, sizeof(command), "powershell -ExecutionPolicy Bypass -File scripts/OpenVSCodeInCurrentFolder.ps1 %lld", (long long)hwnd);
    system(command);
    return true;
}