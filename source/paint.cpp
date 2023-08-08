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

/* HAHAHHA HOW THE FUCK DOES THIS WORK, DON'T TOUCH, SUPER VOLATILE CHATGPT CODE */
void create_window_content(HWND hwnd, float rotation){
    const int padding = 0;

    // Load the bot_layer
    Gdiplus::Image bot_layer(L"gfx\\icon_l1.png");
    Gdiplus::Image top_layer(L"gfx\\icon_l2.png");

    // Create a Bitmap
    Gdiplus::Bitmap bitmap(bot_layer.GetWidth() + padding, bot_layer.GetHeight() + padding, PixelFormat32bppARGB);
    
    // Draw the bot_layer onto the Bitmap
    Gdiplus::Graphics graphics(&bitmap);

    // Set the quality settings for anti-aliasing and high-quality rendering
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
    graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);

    Gdiplus::PointF rotationCenter(bitmap.GetWidth() / 2.0f, bitmap.GetHeight() / 2.0f);

    graphics.TranslateTransform(rotationCenter.X, rotationCenter.Y);
    graphics.RotateTransform(rotation);
    graphics.TranslateTransform(-rotationCenter.X, -rotationCenter.Y);

    graphics.DrawImage(&bot_layer, padding / 2, padding / 2, bot_layer.GetWidth(), bot_layer.GetHeight());

    graphics.TranslateTransform(rotationCenter.X, rotationCenter.Y);
    graphics.RotateTransform(-rotation);
    graphics.TranslateTransform(-rotationCenter.X, -rotationCenter.Y);

    graphics.DrawImage(&top_layer, padding / 2, padding / 2, bot_layer.GetWidth(), bot_layer.GetHeight());

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
    SIZE size = { static_cast<LONG>(bot_layer.GetWidth() + padding), static_cast<LONG>(bot_layer.GetHeight() + padding)};
    POINT pointSource = { 0, 0 };

    // Update the layered window
    UpdateLayeredWindow(hwnd, hdcScreen, NULL, &size, hdc, &pointSource, 0, &bf, ULW_ALPHA);

    // Cleanup
    SelectObject(hdc, hbitmapOld);
    DeleteObject(hbitmap);
    DeleteDC(hdc);
    ReleaseDC(NULL, hdcScreen);
}