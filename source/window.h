#ifndef WINDOW_H
#define WINDOW_H

#include <windows.h>
#include <ShlObj.h>
#include <stdio.h>
#include <stdbool.h>

void  center_window(HWND hwnd);
void  stack_windows(HWND* windows, int width, int height, int xoffset, int yoffset);
void  hide_all_windows();
void  show_desktop_icons(bool visible);
void  show_taskbar(bool visible);
BOOL CALLBACK MinimizeWindowProc(HWND hwnd, LPARAM lParam);

#endif