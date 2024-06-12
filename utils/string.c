#pragma once
#include "types.h"

typedef struct StrBuff
{
    char *content;
    i32 size;
    i32 capacity;
} StrBuff;

inline void StrBuffAppendChar(StrBuff *buff, char ch)
{
    *(buff->content + buff->size) = ch;
    buff->size++;
}

void StrBuffAppendStr(StrBuff *buff, char *str)
{
    while (*str)
    {
        StrBuffAppendChar(buff, *str);
        str++;
    }
}

void StrBuffAppendi32(StrBuff *buff, i32 val)
{
    if (val < 0)
    {
        StrBuffAppendChar(buff, '-');
        val = -val;
    }

    if (val == 0)
        StrBuffAppendChar(buff, '0');

    u32 len = 0;
    char temp[32];
    while (val != 0)
    {
        temp[len++] = '0' + val % 10;
        val /= 10;
    }

    for (i32 i = len - 1; i >= 0; i--)
        StrBuffAppendChar(buff, temp[i]);
}

void StrBuffAppendf32(StrBuff *buff, f32 val, u32 digitsAfterPoint)
{
    StrBuffAppendi32(buff, (i32)val);

    StrBuffAppendChar(buff, '.');
    f32 fraction = val - (i32)val;
    if (fraction < 0)
        fraction = -fraction;
    for (i32 i = 0; i < digitsAfterPoint; i++)
    {
        fraction *= 10;
        StrBuffAppendChar(buff, '0' + (i32)fraction);
        fraction -= (i32)fraction;
    }
}

inline void StrBuffClear(StrBuff *buff)
{
    buff->size = 0;
}

inline void StrBuffSetStr(StrBuff *buff, char *str)
{
    StrBuffClear(buff);
    StrBuffAppendStr(buff, str);
}