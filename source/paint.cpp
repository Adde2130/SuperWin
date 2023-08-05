#include "paint.h"
#include <windows.h>
#include <stdio.h>
#include <gdiplus.h>

static ULONG_PTR gdiplusToken;

void paint_init(){
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    SetProcessDPIAware();
}

void paint_uninit(){
    Gdiplus::GdiplusShutdown(gdiplusToken);
}

void paint_image(HWND hwnd, const WCHAR* filename){
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    //Gdiplus::Graphics graphics(hdc);

    // Load the image
    Gdiplus::Image image(filename);
    Gdiplus::Bitmap bitmap(image.GetWidth(), image.GetHeight(), PixelFormat32bppARGB);

    // Draw the Image onto the Bitmap
    Gdiplus::Graphics graphics(&bitmap);
    graphics.DrawImage(&image, 0, 0, image.GetWidth(), image.GetHeight());

    // Draw the Bitmap onto the window
    Gdiplus::Graphics g(hdc);
    g.DrawImage(&bitmap, 0, 0, bitmap.GetWidth(), bitmap.GetHeight());

    EndPaint(hwnd, &ps);
}

/* HAHAHHA HOW THE FUCK DOES THIS WORK, DON'T TOUCH, SUPER VOLATILE CHATGPT CODE */
void create_window_content(HWND hwnd, const WCHAR* filename){

    // Load the image
    Gdiplus::Image image(filename); // replace with your image path

    // Create a Bitmap
    Gdiplus::Bitmap bitmap(image.GetWidth(), image.GetHeight(), PixelFormat32bppARGB);
    
    // Draw the Image onto the Bitmap
    Gdiplus::Graphics graphics(&bitmap);
    graphics.DrawImage(&image, 0, 0, image.GetWidth(), image.GetHeight());

   // Get the HBITMAP from the Gdiplus::Bitmap
    HBITMAP hbitmap;
    bitmap.GetHBITMAP(Gdiplus::Color(0,0,0,0), &hbitmap);

    // Create a compatible DC
    HDC hdcScreen = GetDC(NULL);
    HDC hdc = CreateCompatibleDC(hdcScreen);

    // Select the HBITMAP into the DC
    HBITMAP hbitmapOld = (HBITMAP)SelectObject(hdc, hbitmap);

    // Initialize the BLENDFUNCTION
    BLENDFUNCTION bf;
    bf.BlendOp = AC_SRC_OVER;
    bf.BlendFlags = 0;
    bf.SourceConstantAlpha = 255; // fully opaque
    bf.AlphaFormat = AC_SRC_ALPHA; // use alpha channel

    // Define size and pointSource
    SIZE size = { static_cast<LONG>(image.GetWidth()), static_cast<LONG>(image.GetHeight()) };
    POINT pointSource = { 0, 0 };

    // Update the layered window
    UpdateLayeredWindow(hwnd, hdcScreen, NULL, &size, hdc, &pointSource, 0, &bf, ULW_ALPHA);

    // Cleanup
    SelectObject(hdc, hbitmapOld);
    DeleteObject(hbitmap);
    DeleteDC(hdc);
    ReleaseDC(NULL, hdcScreen);
}