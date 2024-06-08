#include <windows.h>
#include "types.h"
#include "win32.c"
#include "performance.c"
#include "vec.c"
#include "format.c"
#include "font.c"

#define ONE_OVER_SQUARE_ROOT_OF_TWO 0.70710678118f

bool32 isRunning = 1;
bool32 isFullscreen = 1;

HDC dc;
BITMAPINFO bitmapInfo;
MyBitmap canvas;
HBITMAP bitmap;

// Player info
V2f pos = {20.0f, 20.0f};
f32 speed = 5.0f;

char keys[256] = {0};

inline void PaintRect(MyBitmap *destination, i32 offsetX, i32 offsetY, u32 width, u32 height, u32 color)
{
    if (offsetY < 0)
    {
        height += offsetY;
        offsetY = 0;
    }
    // need to check bounds of the screen
    u32 *row = (u32 *)destination->pixels + offsetX + offsetY * destination->width;
    for (i32 y = 0; y < height; y += 1)
    {
        u32 *pixel = row;
        for (i32 x = 0; x < width; x += 1)
        {
            if ((y + offsetY) >= 0 && (x + offsetX) >= 0 && y + offsetY < destination->height && x + offsetX < destination->width)
                *pixel = color;
            pixel += 1;
        }
        row += destination->width;
    }
}

LRESULT OnEvent(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_QUIT:
    case WM_DESTROY:
        PostQuitMessage(0);
        isRunning = 0;
        break;

    case WM_PAINT:
        PAINTSTRUCT paint;
        BeginPaint(window, &paint);
        EndPaint(window, &paint);
        break;

    case WM_SIZE:
        // appScale = (float)GetDeviceCaps(dc, LOGPIXELSY) / (float)USER_DEFAULT_SCREEN_DPI;
        canvas.width = LOWORD(lParam);
        canvas.height = HIWORD(lParam);
        canvas.bytesPerPixel = 4;

        if (canvas.pixels)
            DeleteObject(bitmap);

        InitBitmapInfo(&bitmapInfo, canvas.width, canvas.height);

        bitmap = CreateDIBSection(dc, &bitmapInfo, DIB_RGB_COLORS, &canvas.pixels, 0, 0);

        SelectObject(dc, bitmap);

        break;

    case WM_KEYDOWN:
        if (wParam < 256)
            keys[wParam] = 1;
        if (wParam == VK_F11)
        {
            isFullscreen = !isFullscreen;
            SetFullscreen(window, isFullscreen);
        }

        break;
    case WM_KEYUP:
        if (wParam < 256)
            keys[wParam] = 0;
        break;
    }
    return DefWindowProc(window, message, wParam, lParam);
}

void WinMainCRTStartup()
{
    InitPerf();

    PreventWindowsDPIScaling();
    dc = CreateCompatibleDC(0);

    InitMyFont(dc);
    HWND window = OpenWindow(OnEvent, 0x222222);
    HDC windowDC = GetDC(window);

    if (isFullscreen)
        SetFullscreen(window, isFullscreen);

    MSG msg;

    while (isRunning)
    {
        StartMetric(Overall);
        while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        memset(canvas.pixels, 0x1c, canvas.bytesPerPixel * canvas.width * canvas.height);

        V2f delta = {0};
        if (keys['W'])
            delta.y = -1.0f;
        else if (keys['S'])
            delta.y = 1.0f;

        if (keys['A'])
            delta.x = -1.0f;
        else if (keys['D'])
            delta.x = 1.0f;

        if (delta.x != 0.0f && delta.y != 0.0f)
            delta = V2fMult(delta, ONE_OVER_SQUARE_ROOT_OF_TWO);

        delta = V2fMult(delta, speed);

        pos = V2fAdd(pos, delta);

        PaintRect(&canvas, pos.x, pos.y, 40, 40, 0xffffff);

        char buff[40];
        i32 len = FormatNumber(GetMicrosecondsFor(Overall) / 1000, buff);
        TextOutA(dc, canvas.width - 80, canvas.height - 40, buff, len);

        BitBlt(windowDC, 0, 0, canvas.width, canvas.height, dc, 0, 0, SRCCOPY);

        Sleep(10); // TODO: proper FPS handling needed, this is just now not to burn CPU
        EndMetric(Overall);
    }

    ExitProcess(0);
}