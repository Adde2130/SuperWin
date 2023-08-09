#include "explorer.h"
#include "window.h"
#include <windows.h>
#include <tchar.h>
#include <shlobj.h>
#include <shldisp.h>
#include <stdbool.h>
#include <stdio.h>

IDispatch* get_explorer(IShellWindows* psw, HWND hwnd){
    HRESULT hr;
    bool free_psw = false;
    if(psw == NULL){
        hr = CoCreateInstance(&CLSID_ShellWindows, NULL, CLSCTX_ALL, &IID_IShellWindows, (void**)&psw);
        free_psw = true;
        if(FAILED(hr)){
            DWORD error = GetLastError();
            fprintf(stderr, "Failed to create COM library instance. Error code: %lu\n", error);
            return NULL;
        }
    }

    long count = 0;
    hr = psw->lpVtbl->get_Count(psw, &count);
    if (FAILED(hr)) {
        if(free_psw)
            psw->lpVtbl->Release(psw);
        fprintf(stderr, "Could not get number of shell windows\n");
        return NULL;
    }

    for (long i = 0; i < count; ++i) {
        VARIANT vi;
        VariantInit(&vi);
        vi.vt = VT_I4;
        vi.lVal = i;
        IDispatch *pDisp = NULL;
        hr = psw->lpVtbl->Item(psw, vi, &pDisp);
        VariantClear(&vi);

        if (FAILED(hr) || !pDisp)
            continue;

        IWebBrowserApp *pApp = NULL;
        hr = pDisp->lpVtbl->QueryInterface(pDisp, &IID_IWebBrowserApp, (void**)&pApp);
        pApp->lpVtbl->Release(pApp);
        if (FAILED(hr))
            continue;

        HWND temphwnd;
        pApp->lpVtbl->get_HWND(pApp, (SHANDLE_PTR*)&temphwnd);
        if(temphwnd != hwnd)
            continue;

        char classname[256];
        GetClassName(hwnd, classname, sizeof(classname));
        if(strcmp(classname, "CabinetWClass"))
            continue;

        if(free_psw)
            psw->lpVtbl->Release(psw);

        return pDisp;

    }

    psw->lpVtbl->Release(psw);

    return NULL;
}

char* get_explorer_path(HWND hwnd){
    HWND tophwnd = GetForegroundWindow();
    if(tophwnd == NULL) return NULL;

    char classname[256];
    GetClassName(hwnd, classname, sizeof(classname));
    if(strcmp(classname, "CabinetWClass"))
        return NULL;

    IShellWindows *psw = NULL;
    HRESULT hr = CoCreateInstance(&CLSID_ShellWindows, NULL, CLSCTX_ALL, &IID_IShellWindows, (void**)&psw);

    if (FAILED(hr)) {
        fprintf(stderr, "Could not create instance of IShellWindows\n");
        return NULL;
    }

    long count = 0;
    hr = psw->lpVtbl->get_Count(psw, &count);

    if (FAILED(hr)) {
        fprintf(stderr, "Could not get number of shell windows\n");
        return NULL;
    }

    IDispatch* pdisp = get_explorer(psw, hwnd);

    IServiceProvider *psp = NULL;
    hr = pdisp->lpVtbl->QueryInterface(pdisp, &IID_IServiceProvider, (void**)&psp);
    if (FAILED(hr) || !psp){
        fprintf(stderr, "Failed to query IServiceProvider interface\n");
        return NULL;
    }
        
    IShellBrowser *pBrowser = NULL;
    hr = psp->lpVtbl->QueryService(psp, &SID_STopLevelBrowser, &IID_IShellBrowser, (void**)&pBrowser);
    if (FAILED(hr)){
        fprintf(stderr, "Failed to query shell browser\n");
        return NULL;
    }
        

    IShellView *pShellView = NULL;
    hr = pBrowser->lpVtbl->QueryActiveShellView(pBrowser, &pShellView);
    if (FAILED(hr)){
        fprintf(stderr, "Failed to query Shell View\n");
        return NULL;
    }
        

    IFolderView *pFolderView = NULL;
    hr = pShellView->lpVtbl->QueryInterface(pShellView, &IID_IFolderView, (void**)&pFolderView);
    if (FAILED(hr) || !pFolderView){
        fprintf(stderr, "Failed to query folder view\n");
        return NULL;
    }
        

    IPersistFolder2 *pFolder = NULL;
    hr = pFolderView->lpVtbl->GetFolder(pFolderView, &IID_IPersistFolder2, (void**)&pFolder);
    if (FAILED(hr)){
        fprintf(stderr, "Failed to query folder\n");
        return NULL;
    }
        

    LPITEMIDLIST pidl = NULL;
    hr = pFolder->lpVtbl->GetCurFolder(pFolder, &pidl);
    if (!SUCCEEDED(hr)) {
        fprintf(stderr, "Failed to get current folder PIDL\n");
        return NULL;
    }

    TCHAR szPath[MAX_PATH];
    if (SHGetPathFromIDList(pidl, szPath))
    {
        CoTaskMemFree(pidl);
        return strdup(szPath);
    }
    else
    {
        fprintf(stderr, "Failed to convert PIDL to path.\n");
    }
    // Free the PIDL when done.
    CoTaskMemFree(pidl);

    return NULL;
}

HWND get_explorer_from_path(const char* path){
    IShellWindows *psw = NULL;
    HRESULT hr = CoCreateInstance(&CLSID_ShellWindows, NULL, CLSCTX_ALL, &IID_IShellWindows, (void**)&psw);

    if (FAILED(hr)) {
        fprintf(stderr, "Could not create instance of IShellWindows\n");
        return NULL;
    }

    long count = 0;
    hr = psw->lpVtbl->get_Count(psw, &count);

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
        hr = psw->lpVtbl->Item(psw, vi, &pDisp);

        if (FAILED(hr) || !pDisp)
            continue;

        IWebBrowserApp *pApp = NULL;
        hr = pDisp->lpVtbl->QueryInterface(pDisp, &IID_IWebBrowserApp, (void**)&pApp);

        if (FAILED(hr) || !pApp)
            continue;

        HWND hwnd;
        pApp->lpVtbl->get_HWND(pApp, (SHANDLE_PTR*)&hwnd);
        if(!hwnd)
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
                if(strcmp(szPath, path) == 0) {
                    CoTaskMemFree(pidl);
                    return hwnd;
                } 
            }
            else
            {
                fprintf(stderr, "Failed to convert PIDL to path.\n");
            }
            // Free the PIDL when done.
            CoTaskMemFree(pidl);
        }
    }

    return NULL;
}

HWND* get_all_explorer_windows(IShellWindows* psw){
    HRESULT hr;
    bool free_psw = false;
    if(psw == NULL){
        hr = CoCreateInstance(&CLSID_ShellWindows, NULL, CLSCTX_ALL, &IID_IShellWindows, (void**)&psw);
        free_psw = true;
        if(FAILED(hr)) {
            fprintf(stderr, "Failed to create COM library instance\n");
            return NULL;
        }
    }

    long count = 0;
    hr = psw->lpVtbl->get_Count(psw, &count);
    if (FAILED(hr)) {
        fprintf(stderr, "Could not get number of shell windows\n");
        if(free_psw)
            psw->lpVtbl->Release(psw);
        return NULL;
    }

    int size = 0;
    HWND* windows = malloc(sizeof(HWND) * count);

    for (long i = 0; i < count; ++i) {
        VARIANT vi;
        VariantInit(&vi);
        vi.vt = VT_I4;
        vi.lVal = i;
        IDispatch *pDisp = NULL;
        hr = psw->lpVtbl->Item(psw, vi, &pDisp);
        VariantClear(&vi);
        if (FAILED(hr) || !pDisp)
            continue;

        IWebBrowserApp* pApp;
        hr = pDisp->lpVtbl->QueryInterface(pDisp, &IID_IWebBrowserApp, (void**)&pApp);
        pDisp->lpVtbl->Release(pDisp);
        if(FAILED(hr) || !pApp)
            continue;

        HWND hwnd;
        hr = pApp->lpVtbl->get_HWND(pApp, (SHANDLE_PTR*)&hwnd);
        pApp->lpVtbl->Release(pApp);
        if(FAILED(hr) || !hwnd)
            continue;

        char classname[256];

        GetClassName(hwnd, classname, sizeof(classname));
        if(strcmp(classname, "CabinetWClass"))
            continue;
        
        windows[size] = hwnd;       
        size++;
    }

    if(size == 0) {
        free(windows);
        return NULL;
    }

    if(free_psw){
        psw->lpVtbl->Release(psw);
    }

    // Error prone apparently?
    windows = realloc(windows, sizeof(HWND) * (size) + 1);
    windows[size] = NULL;

    return windows;
}

bool explorer_change_content(IShellWindows* psw, HWND hwnd, const char* path){
    HRESULT hr;
    bool free_psw = false;
    if(psw == NULL){
        hr = CoCreateInstance(&CLSID_ShellWindows, NULL, CLSCTX_ALL, &IID_IShellWindows, (void**)&psw);
        free_psw = true;
        if(FAILED(hr)) {
            fprintf(stderr, "Failed to create COM library instance\n");
            return false;
        }
    }


    char classname[256];
    GetClassName(hwnd, classname, sizeof(classname));
    if(strcmp(classname, "CabinetWClass")){
        if(free_psw)
            psw->lpVtbl->Release(psw);
        return false;
    }
        
    IDispatch* pdisp = get_explorer(psw, hwnd);  
    if(free_psw)
        psw->lpVtbl->Release(psw);

    if(pdisp == NULL){
        return false;
    }

    IServiceProvider* psp;
    hr = pdisp->lpVtbl->QueryInterface(pdisp, &IID_IServiceProvider, (void**)&psp);
    pdisp->lpVtbl->Release(pdisp);
    if(FAILED(hr) || !psp){
        fprintf(stderr, "Failed to query ISeriveProvider\n");
        return false;
    }

    IShellBrowser* psb;
    hr = psp->lpVtbl->QueryService(psp, &SID_STopLevelBrowser, &IID_IShellBrowser, (void**)&psb);
    if(FAILED(hr) || !psb){
        fprintf(stderr, "Failed to query shell browser\n");
        psp->lpVtbl->Release(psp);
        return false;
    }
    psp->lpVtbl->Release(psp);


    LPITEMIDLIST pidl = ILCreateFromPath(_T(path));
    if (pidl)
    {
        hr = psb->lpVtbl->BrowseObject(psb, pidl, SVSI_DESELECTOTHERS | SVSI_SELECT | SVSI_ENSUREVISIBLE | SVSI_FOCUSED | SVSI_DESELECT);
        psb->lpVtbl->Release(psb);
        ILFree(pidl);
        if(FAILED(hr)){
            fprintf(stderr, "Failed to browse object\n");
            return false;
        }
        return true;
    }
    psb->lpVtbl->Release(psb);

    return false;
}

void add_explorer_path(int preset){
    char* path = get_explorer_path(GetForegroundWindow());
    if(path == NULL)
        return;
    
    FILE* f = fopen("cfg/exp_presets", "r");
    if(f == NULL) {
        fprintf(stderr, "WARNING: exp_presets missing\n");
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
        fprintf(stderr, "WARNING: exp_presets missing\n");
        return;
    }

    char path[256] = {0};
    int line = 0;
    while(fgets(path, sizeof(path), f)){
        line++;
        if(line == preset)
            break;
    }
    path[strcspn(path, "\n")] = 0;

    fclose(f);
    if( GetAsyncKeyState(VK_SHIFT) >> 15){
        ShellExecute(NULL, "open", "explorer.exe", path, NULL, SW_SHOWDEFAULT);
        return;
    }

    HWND hwnd = get_explorer_from_path(path);
    if(hwnd) {
    /*  Little cheat to bypass the SetForegroundWindow restriction used when the function is called too often.
        This potentially causes problems, since it will press control for the user, and thus may accidentally input
        something unintentional. This should not the case though since it only happens when an explorer is focused. */
        keybd_event(VK_CONTROL, 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
        keybd_event(VK_CONTROL, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);

        BringWindowToTop(hwnd);
        SetFocus(hwnd);
        SetForegroundWindow(hwnd);
        return;
    }

    if(!explorer_change_content(NULL, GetForegroundWindow(), path))
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

    char classname[256];
    GetClassName(hwnd, classname, sizeof(classname));
    if(strcmp(classname, "CabinetWClass"))
        return false;
    
    char command[256];
    snprintf(command, sizeof(command), "powershell -ExecutionPolicy Bypass -File scripts/OpenVSCodeInCurrentFolder.ps1 %lld", (long long)hwnd);
    system(command);
    return true;
}

void center_explorers(){
    HWND* windows = get_all_explorer_windows(NULL);
    if(windows == NULL) return;
    for(int i = 0; windows[i]; i++) {
        center_window(windows[i]);
    }
}