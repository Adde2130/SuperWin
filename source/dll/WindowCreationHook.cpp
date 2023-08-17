#include <windows.h>
#include <set>
#include <vector>
#include <algorithm>
#include <string>

std::set<HWND> encountered_windows;
std::set<HWND> windows_on_screen;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    (void)lParam;
    encountered_windows.insert(hwnd);
    return TRUE;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    (void)hModule;
    (void)lpReserved;
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        if(encountered_windows.size() == 0)
            EnumWindows(EnumWindowsProc, 0);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

extern "C" __declspec(dllexport) LRESULT CALLBACK CBTHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    switch(nCode) {
    case HCBT_ACTIVATE: {
        HWND hwnd = (HWND)wParam;
        if (encountered_windows.count(hwnd))
            break;

        SetWindowPos(hwnd, NULL, 960 * (windows_on_screen.size() % 2), 540 * (windows_on_screen.size() / 2), 0, 0, SWP_NOZORDER | SWP_NOSIZE);
        encountered_windows.insert(hwnd);
        windows_on_screen.insert(hwnd);

        break; 
    }

    case HCBT_DESTROYWND: {
        HWND hwnd = (HWND)wParam;
        windows_on_screen.erase(hwnd);
        encountered_windows.erase(hwnd);
        break;
    }

    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);

}