#ifndef EXPLORER_H
#define EXPLORER_H

#include <stdbool.h>
#include <windows.h>
#include <ShlObj.h>

//------------------------------------------------------------------------
// CURRENT IDEAS: 
//         -HOTKEYS FOR DIRECTORIES
//                  -If SHIFT+any number is pressed, the currently selected
//                   directory gets bound to that number.
//                  -If there is a directory focused and a number is
//                   pressed, the directory changes its to the bound path
//                  -If there is not a directory focused/open and a number
//                   is pressed, then open a directory with the bound path
//------------------------------------------------------------------------

IDispatch* get_explorer(IShellWindows* psw, HWND hwnd);
char*      get_explorer_path(HWND hwnd);
void       add_explorer_path(int preset);
void       open_explorer(int preset);
void       close_explorer();
bool       open_vscode();
bool       explorer_change_content(HWND hwnd, const char* path);
void       center_explorers();
HWND*      get_all_explorer_windows(IShellWindows* psw);

#endif