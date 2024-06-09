#pragma once
#include <windows.h>
#include "utils/types.h"

HFONT consolasFont;

void InitMyFont(HDC dc)
{
    i32 fontHeight = MulDiv(15, GetDeviceCaps(dc, LOGPIXELSY), USER_DEFAULT_SCREEN_DPI);
    consolasFont = CreateFontA(-fontHeight, 0, 0, 0,
                               FW_NORMAL, // Weight
                               0,         // Italic
                               0,         // Underline
                               0,         // Strikeout
                               DEFAULT_CHARSET,
                               OUT_TT_ONLY_PRECIS,
                               CLIP_DEFAULT_PRECIS,

                               // I've experimented with the Chrome and it doesn't render LCD quality for fonts above 32px
                               CLEARTYPE_QUALITY,

                               DEFAULT_PITCH,
                               "Consolas");

    SelectObject(dc, consolasFont);

    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, 0xffffff);
    // GetTextMetricsA(dc, &tm);
}
