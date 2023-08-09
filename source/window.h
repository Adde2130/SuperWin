#ifndef WINDOW_H
#define WINDOW_H

#include <windows.h>
#include <ShlObj.h>
#include <stdio.h>

void  center_window(HWND hwnd);
void  stack_windows(HWND* windows, int width, int height, int xoffset, int yoffset);

#endif