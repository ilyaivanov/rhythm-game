#include "windows.h"
#include "utils/types.h"

// char *fontPath = "c:/windows/fonts/cour.ttf";
char *fontPath = "c:/windows/fonts/segoeui.ttf";
// char *fontPath = "c:/windows/fonts/lucon.ttf";

// char *fontName = "Courier New";
char *fontName = "Segoe UI";

// monospaced
//  char *fontName = "Lucida Console";

MyBitmap textures[256];

// Kerning? Never heard of that.
u8 widths[256];

TEXTMETRIC textMetric;

void InitFontSystem(int fontSize)
{
    // TODO: do I need to release this context
    HDC deviceContext = CreateCompatibleDC(0);

    // AddFontResourceExA(fontPath, FR_PRIVATE, 0);

    int textureSize = 256;
    int h = -MulDiv(fontSize, GetDeviceCaps(deviceContext, LOGPIXELSY), 72);
    HFONT font = CreateFontA(h, 0, 0, 0,
                             FW_DONTCARE, // Weight
                             0,           // Italic
                             0,           // Underline
                             0,           // Strikeout
                             DEFAULT_CHARSET,
                             OUT_TT_ONLY_PRECIS,
                             CLIP_DEFAULT_PRECIS,
                             CLEARTYPE_QUALITY,
                             DEFAULT_PITCH,
                             fontName);

    BITMAPINFO info = {
        .bmiHeader = {
            .biSize = sizeof(info.bmiHeader),
            .biBitCount = 32,
            .biCompression = BI_RGB,
            .biWidth = textureSize,
            .biHeight = -textureSize,
            .biPlanes = 1,
        }};

    void *bits;
    HBITMAP bitmap = CreateDIBSection(deviceContext, &info, DIB_RGB_COLORS, &bits, 0, 0);

    SelectObject(deviceContext, bitmap);
    SelectObject(deviceContext, font);

    SetBkColor(deviceContext, RGB(0, 0, 0));
    SetTextColor(deviceContext, RGB(255, 255, 255));

    SIZE size;
    GetTextMetrics(deviceContext, &textMetric);

    for (char ch = 32; ch <= 126; ch += 1)
    {
        int len = 1;
        GetTextExtentPoint32A(deviceContext, &ch, len, &size);

        widths[ch - 32] = size.cx;
        TextOutA(deviceContext, 0, 0, &ch, len);

        MyBitmap *texture = &textures[ch - 32];
        texture->width = size.cx;
        texture->height = size.cy;
        texture->bytesPerPixel = 4;

        texture->pixels = malloc(texture->height * texture->width * texture->bytesPerPixel);

        u32 *row = (u32 *)texture->pixels;
        u32 *source = (u32 *)bits;
        for (u32 y = 0; y < texture->height; y += 1)
        {
            u32 *pixel = row;
            u32 *sourcePixel = source;
            for (u32 x = 0; x < texture->width; x += 1)
            {
                *pixel = *sourcePixel;
                sourcePixel += 1;
                pixel += 1;
            }
            source += textureSize;
            row += texture->width;
        }
    }

    DeleteDC(deviceContext);
}

MyBitmap *GetGlyphBitmap(char codepoint)
{
    MyAssert(codepoint >= 32 && codepoint <= 126);
    return &textures[codepoint - 32];
}

u8 GetGlyphWidth(char codepoint)
{
    MyAssert(codepoint >= 32 && codepoint <= 126);
    return widths[codepoint - 32];
}

int GetFontHeight()
{
    return textMetric.tmHeight + textMetric.tmExternalLeading;
}