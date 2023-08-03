#ifndef EXPLORER_H
#define EXPLORER_H

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

char* get_focused_explorer_path();
void  add_explorer_path(int preset);
void  open_explorer(int preset);
void  close_explorer();
void  open_vscode();

#endif