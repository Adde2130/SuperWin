#include <windows.h>
#include <tchar.h>
#include <shlobj.h>
#include <shldisp.h>
#include <wincodec.h>
#include <stdbool.h>
#include <stdio.h>

#include "explorer.h"
#include "paint.h"

// This is a custom command
#define WM_EXITAPP WM_USER + 1
#define downtime(key) (GetTickCount() - keys[key].pressed_time)

bool super_mode = true;

typedef struct {
    DWORD pressed_time;
    bool down;
} KeyState;

KeyState keys[256] = {0};

HWND hwnd;

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
            ShowWindow(hwnd, super_mode);
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

        case 'K':
            return 1;

        case VK_INSERT: //ESC KEY
            if(downtime(VK_INSERT) > 1000) {
                PostMessage(NULL, WM_EXITAPP, 0, 0);
                return 1;
            }
            if(downtime(VK_INSERT) > 20)
                return 1;
            super_mode = false;
            ShowWindow(hwnd, super_mode);
            return 1;

        case 'Q':
            close_explorer();
            break;
        
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            if(GetAsyncKeyState(VK_SHIFT) >> 15)
                add_explorer_path(key - 48);
            else
                open_explorer(key - 48);
            return 1;
        }
    }

    return CallNextHookEx(NULL, n_code, w_param, l_param);
}

// Callbacks to the visible part
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
    case WM_PAINT: {
        return 0;
    }

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

    hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        "Notifier",
        "SuperMode Active",
        WS_POPUP,
        1750, 50,
        200, 200,
        NULL, NULL, hInstance, NULL
    );

    create_window_content(hwnd, L"gfx/icon.png");

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
    if(CoInitializeEx(NULL, COINIT_MULTITHREADED) != S_OK){
        printf("ERROR: Failed to initialize the COM library");
        return 1;
    }
    paint_init();
    

    create_window(hInstance, nCmdShow);

    HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, hInstance, 0);

    // Ok apparently this is a way to tell the compiler that I know that they are unused but it doesn't matter
    (void)hPrevInstance;
    (void)lpCmdLine;

    if (!hook) {
        MessageBox(NULL, "WINDOW HOOK FAILED????", "Error", MB_ICONERROR);
        return 1;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        if(msg.message == WM_EXITAPP)
            break;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    paint_uninit();
    UnhookWindowsHookEx(hook);
    CoUninitialize();
    return msg.wParam;
}