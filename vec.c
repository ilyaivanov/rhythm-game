#pragma once
#include "types.h"
#include "xmmintrin.h"

typedef struct V2f
{
    f32 x, y;
} V2f;

inline float squareRoot(const float val)
{
    return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(val)));
}

f32 V2fDistance(V2f point1, V2f point2)
{
    f32 dx = point1.x - point2.x;
    f32 dy = point1.y - point2.y;
    return squareRoot(dx * dx + dy * dy);
}

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