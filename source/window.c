#include "window.h"

#include <stdbool.h>

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

// ChatGPT code
void show_desktop_icons(bool visible){
    HWND progman = FindWindow("Progman", NULL);
    if (!progman) return;

    HWND listView = 0;
    HWND shellViewWin = FindWindowEx(progman, NULL, "SHELLDLL_DefView", NULL);
    if (shellViewWin) {
        listView = FindWindowEx(shellViewWin, NULL, "SysListView32", NULL);        
    }

    if(!listView) {
        HWND workerW = NULL;
        while ((workerW = FindWindowEx(NULL, workerW, "WorkerW", NULL)) != NULL) {
            shellViewWin = FindWindowEx(workerW, NULL, "SHELLDLL_DefView", NULL);
            if (shellViewWin) {
                listView = FindWindowEx(shellViewWin, NULL, "SysListView32", NULL);
            }
        }
    }

    if(!listView) return;
    
    ShowWindow(listView, visible ? SW_SHOW : SW_HIDE);

    // Force refresh the desktop
    SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, NULL, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);

}

void show_taskbar(bool visible){
    HWND taskbar = FindWindow("Shell_TrayWnd", NULL);
    if (taskbar)
        ShowWindow(taskbar, visible ? SW_SHOW : SW_HIDE);
}

void stack_windows(HWND* windows, int width, int height, int xoffset, int yoffset){
    if(windows == NULL) return;

    int size = 0;
    for(;windows[size]; size++); // Hate this

    RECT scr_dim;

    scr_dim.left   = GetSystemMetrics(SM_XVIRTUALSCREEN);
    scr_dim.top    = GetSystemMetrics(SM_YVIRTUALSCREEN);
    scr_dim.right  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    scr_dim.bottom = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    int startx = (scr_dim.right - width) / 2 - (((size - 1) * xoffset) / 2);
    int starty = (scr_dim.bottom - height) / 2 - (((size - 1) * yoffset) / 2);
    for(int i = size - 1; i >= 0; i--){
        bool success = SetWindowPos(windows[i], windows[i + 1], startx + xoffset * i, starty + yoffset * i, width, height, 0);
        if(!success) {
            DWORD error = GetLastError();
            fprintf(stderr, "Failed to reposition window. Error code: %lu", error);
            printf(", i = %d \n", i);
            continue;
        }

        if(i < size - 1)
            continue;

        success = SetForegroundWindow(windows[i]);
        if(!success) {
            DWORD error = GetLastError();
            if(GetLastError())
                fprintf(stderr, "Failed to bring window to top. Error code: %lu\n", error);
        }

    }

}
