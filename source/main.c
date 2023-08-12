#include <windows.h>
#include <tchar.h>
#include <shlobj.h>
#include <shldisp.h>
#include <wincodec.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef WIN32
#include <io.h>
#define F_OK 0
#define access _access
#endif

#include "explorer.h"
#include "paint.h"
#include "window.h"

// This is a custom command
#define WM_EXITAPP WM_USER + 1

#define downtime(key) (GetTickCount() - keys[key].pressed_time)

typedef struct {
    DWORD pressed_time;
    bool down;
} KeyState;


HWND windows[4] = {0,0,0,0};

bool super_mode = true;

float r = 0; // Rotation of circle
float speed = 1.5; // Think it is in radians


KeyState keys[256] = {0};

HWND hwnd;
int wx, wy;

int tick = 0;

bool animate_in = true;

void setup_windows(){
    HWND* new_windows = get_all_explorer_windows(NULL);
    windows[0] = new_windows[0];
    windows[1] = new_windows[1];
    windows[2] = new_windows[2];
    windows[3] = new_windows[3];
    free(new_windows);

    int screenw = GetSystemMetrics(SM_CXSCREEN);
    int screenh = GetSystemMetrics(SM_CYSCREEN);

    int posx = 0;
    int posy = 0;

    for(int i = 0; i < 4; i++){
        HWND hwnd = windows[i];
        if(hwnd == NULL) 
            continue;

        ShowWindow(hwnd, SW_RESTORE);
        LONG style = GetWindowLong(hwnd, GWL_STYLE);
        SetWindowLong(hwnd, GWL_STYLE, style & ~WS_CAPTION);

        MoveWindow(hwnd, posx, posy, screenw / 2, screenh / 2, true);
        posx += screenw / 2;
        if(posx >= screenw){
            posx = 0;
            posy += screenh / 2;
        }
    }
}

void enter_super_mode(){
    show_desktop_icons(false);
    show_taskbar(false);
    //setup_windows();
}

void exit_super_mode(){
    show_desktop_icons(true);
    show_taskbar(true);
}

void animate_window(HWND hwnd){
    int curve_pos = wx + 225 + tick * tick - 30 * tick;
    int curve_speed = 2 * tick - 30;
    
    if(animate_in && curve_speed < 0)
        tick++;
    else if(!animate_in && tick > 0)
        tick--;
    else return;

    SetWindowPos(hwnd, NULL, curve_pos, wy, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

LRESULT CALLBACK LowLevelKeyboardProc(int n_code, WPARAM w_param, LPARAM l_param){
    if(n_code != HC_ACTION)
        return CallNextHookEx(NULL, n_code, w_param, l_param);

    // What the actual fuck
    PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)l_param;

    /* HANDLE KEY STATES */
    if(w_param == WM_KEYDOWN && !keys[p->vkCode].down){
        keys[p->vkCode].pressed_time = GetTickCount();
        keys[p->vkCode].down = true;
    } else if(w_param == WM_KEYUP && keys[p->vkCode].down){
        keys[p->vkCode].pressed_time = 0;
        keys[p->vkCode].down = false;
    }

    if(w_param == WM_KEYDOWN){
        if(!super_mode){
            if(p->vkCode != VK_INSERT)
                return CallNextHookEx(NULL, n_code, w_param, l_param);
            if(downtime(VK_INSERT) > 1000){
                PostMessage(NULL, WM_EXITAPP, 0, 0);
                return 1;
            }
            if(downtime(VK_INSERT) > 20)
                return 1;
            super_mode = true;
            animate_in = true;
            SetWindowPos(hwnd, NULL, 2000, wy, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
            ShowWindow(hwnd, super_mode);
            enter_super_mode();
            return 1;
        }


        DWORD key = p->vkCode;
        switch(key) {
        case 'C':
            if(GetAsyncKeyState(VK_CONTROL) >> 15) // MOST SIGNIFICANT BIT IS IF KEY IS DOWN
                return CallNextHookEx(NULL, n_code, w_param, l_param);
            if(!open_vscode())
                return CallNextHookEx(NULL, n_code, w_param, l_param);
            return 1;

        case 'F':
            center_explorers();
            return 1;

        case 'Q':
            close_explorer();
            return 1;

        case 'R': {
            HWND* windows = get_all_explorer_windows(NULL);
            stack_windows(windows, 1040, 620, 40, 40);
            if(windows != NULL)
                free(windows);
            }
            return 1;
        
        case 'E': {
            HWND* windows = get_all_explorer_windows(NULL);
            stack_windows(windows, 1040, 620, 40, 0);
            if(windows != NULL)
                free(windows);
            }
            return 1;

        case 'M':
            
            return 1;
        
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            if(GetAsyncKeyState(VK_CONTROL) >> 15)
                add_explorer_path(key - 48);
            else
                open_explorer(key - 48);
            return 1;

        case VK_INSERT: //ESC KEY
            if(downtime(VK_INSERT) > 1000) {
                PostMessage(NULL, WM_EXITAPP, 0, 0);
                return 1;
            }
            if(downtime(VK_INSERT) > 20)
                return 1;
            super_mode = false;
            animate_in = false;
            exit_super_mode();
            return 1;
        }
    }

    return CallNextHookEx(NULL, n_code, w_param, l_param);
}

LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam){
    if (nCode == HCBT_CREATEWND) {
        HWND hwnd = (HWND)wParam;

        // Adjust size and position
        SetWindowPos(hwnd, NULL, 960, 540, 960, 540, SWP_NOZORDER);

        // Remove the title bar
        LONG style = GetWindowLong(hwnd, GWL_STYLE);
        SetWindowLong(hwnd, GWL_STYLE, style & ~WS_CAPTION);
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// Callbacks to the visible part
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
    case WM_CREATE:
        SetTimer(hwnd, 1, 1, NULL);
        return 0;
    case WM_TIMER:
        create_window_content(hwnd, r);
        r+=speed;
        animate_window(hwnd);
        DWORD exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
        if((exStyle & WS_EX_TOPMOST) != 0)
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        return 0;
    case WM_PAINT:
        return 0;

    /* MAKES CLICKS GO THROUGH */
    case WM_NCHITTEST:
        return HTTRANSPARENT;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}



void create_window(HINSTANCE hInstance, int nCmdShow){
    WNDCLASS window = {}; // Empty init makes compiler not throw warnings
    window.lpfnWndProc = WndProc;
    window.hInstance = hInstance;
    window.lpszClassName = "Notifier";
    window.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    RegisterClass(&window);

    wx = 1750;
    wy = 50;

    hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        "Notifier",
        "SuperMode Active",
        WS_POPUP,
        2000, wy,
        250, 250,
        NULL, NULL, hInstance, NULL
    );

    create_window_content(hwnd, 0.0f);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
}

void fix_files(){
    if (!CreateDirectory("cfg", NULL)) {
        DWORD error = GetLastError();
        if (error != ERROR_ALREADY_EXISTS) {
            printf("Failed to create directory. Error code: %lu\n", error);
            return;
        }
    }

    if(access("cfg\\exp_presets", F_OK)) { // returns 0 if file exists
        FILE* f = fopen("cfg\\exp_presets", "w");

        if(!f) {
            DWORD error = GetLastError();
            printf("Failed to create file. Error code: %lu\n", error);
        } else {
            for(int i = 0; i < 9; i++)
                fprintf(f, "C:\\\n");
            fclose(f);
        }
    }
    
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){

    // Tell the compiler they aren't used
    (void)hPrevInstance;
    (void)lpCmdLine;

    fix_files();

    if(CoInitializeEx(NULL, COINIT_MULTITHREADED) != S_OK){
        printf("ERROR: Failed to initialize the COM library");
        return 1;
    }
    paint_init();

    create_window(hInstance, nCmdShow);

    HHOOK key_hook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, hInstance, 0);
    if (!key_hook) {
        MessageBox(NULL, "KEY_HOOK FAILED", "Error", MB_ICONERROR);
        return 1;
    }

    HHOOK cbt_hook = SetWindowsHookEx(WH_CBT, CBTProc, NULL, GetCurrentThreadId());
    if (!cbt_hook) {
        MessageBox(NULL, "CBT_HOOK FAILED", "Error", MB_ICONERROR);
        return 1;
    }

    enter_super_mode();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        if(msg.message == WM_EXITAPP)
            break;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    exit_super_mode();
    paint_uninit();
    UnhookWindowsHookEx(key_hook);
    UnhookWindowsHookEx(cbt_hook);
    CoUninitialize();
    return msg.wParam;
}
