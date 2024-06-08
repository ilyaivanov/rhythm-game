#pragma once

#include <stdint.h>
#include <windows.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;

typedef float f32;
typedef double f64;

typedef i32 bool32;

typedef struct MyBitmap
{
    u32 width;
    u32 height;
    u32 bytesPerPixel;
    u32 *pixels;
} MyBitmap;

typedef struct V2i
{
    i32 x, y;
} V2i;
typedef struct V2f
{
    f32 x, y;
} V2f;

typedef struct V3f
{
    f32 x, y, z;
} V3f;

typedef struct Mat4
{
    float values[16];
} Mat4;

#define Kilobytes(val) (val * 1024)
#define Megabytes(val) Kilobytes(val * 1024)
#define Gigabytes(val) Megabytes(val * 1024)

#define ArrayLength(array) (sizeof(array) / sizeof(array[0]))
#define Assert(cond)   \
    if (!(cond))       \
    {                  \
        *(u32 *)0 = 0; \
    }

#define Fail(msg) Assert(0)

inline V2f V2fMult(V2f p, f32 scalar)
{
    return (V2f){
        p.x * scalar,
        p.y * scalar,
    };
}

inline V2f V2fAdd(V2f p1, V2f p2)
{
    return (V2f){
        p1.x + p2.x,
        p1.y + p2.y,
    };
}