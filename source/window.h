#ifndef WINDOW_H
#define WINDOW_H

#include <windows.h>
#include <ShlObj.h>
#include <stdio.h>

void center_window(HWND hwnd);
HWND* get_all_explorer_windows(IShellWindows* psw);

#endif