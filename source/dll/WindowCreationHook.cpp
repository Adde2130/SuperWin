#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

extern "C" __declspec(dllexport) LRESULT CALLBACK CBTHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HCBT_CREATEWND) {
        MessageBoxA(NULL, "HERE", "HERE", MB_OK);
        HWND hwnd = (HWND)wParam;

        // Adjust size and position
        SetWindowPos(hwnd, NULL, 960, 540, 960, 540, SWP_NOZORDER);

        // Remove the title bar
        LONG style = GetWindowLong(hwnd, GWL_STYLE);
        SetWindowLong(hwnd, GWL_STYLE, style & ~WS_CAPTION);
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}