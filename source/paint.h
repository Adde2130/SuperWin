#ifndef PAINT_H
#define PAINT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

void paint_init();
void paint_uninit();
void paint_image(HWND hwnd, const WCHAR* filename);
void create_window_content(HWND hwnd, const WCHAR* filename);

#ifdef __cplusplus
}
#endif

#endif