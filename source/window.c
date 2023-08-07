#include "window.h"

HWND* get_all_explorer_windows(IShellWindows* psw){
    HRESULT hr;
    if(psw == NULL){
        hr = CoCreateInstance(&CLSID_ShellWindows, NULL, CLSCTX_ALL, &IID_IShellWindows, (void**)&psw);
        if(FAILED(hr)) {
            fprintf(stderr, "Failed to create COM library instance\n");
            return NULL;
        }
    }

    long count = 0;
    hr = psw->lpVtbl->get_Count(psw, &count);
    if (FAILED(hr)) {
        fprintf(stderr, "Could not get number of shell windows\n");
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

        if (FAILED(hr) || !pDisp)
            continue;

        IWebBrowserApp* pApp;
        hr = pDisp->lpVtbl->QueryInterface(pDisp, &IID_IWebBrowserApp, (void**)&pApp);
        if(FAILED(hr) || !pApp)
            continue;

        HWND hwnd;
        hr = pApp->lpVtbl->get_HWND(pApp, (SHANDLE_PTR*)&hwnd);
        if(FAILED(hr) || !hwnd)
            continue;

        char classname[256];
        GetClassName(hwnd, classname, sizeof(classname));
        if(strcmp(classname, "CabinetWClass"))
            continue;
        
        windows[size++] = hwnd;       

    }

    if(size == 0){
        free(windows);
        return NULL;
    }

    windows[size++] = NULL;
    windows = realloc(windows, sizeof(HWND) * size);

    return windows;
}

void center_window(HWND hwnd){
    RECT win_dim, scr_dim;
    GetWindowRect(hwnd, &win_dim);
    int width = win_dim.right - win_dim.left;
    int height = win_dim.bottom - win_dim.top;

    scr_dim.left   = GetSystemMetrics(SM_XVIRTUALSCREEN);
    scr_dim.top    = GetSystemMetrics(SM_YVIRTUALSCREEN);
    scr_dim.right  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    scr_dim.bottom = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    int x = (scr_dim.right - width) / 2;
    int y = (scr_dim.bottom - height) / 2;

    SetWindowPos(hwnd, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}