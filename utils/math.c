#pragma once
#include "types.h"
#include "xmmintrin.h"

// Floats

inline f32 Clamp(f32 val, f32 min, f32 max)
{
    if (val > max)
        return max;
    if (val < min)
        return min;
    return val;
}

inline float Sqrt(const float val)
{
    return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(val)));
}

//
// Vectors
//
//

// clang-format off

typedef struct V2f { f32 x, y;    } V2f;
typedef struct V2i { i32 x, y;    } V2i;
typedef struct V3f { f32 x, y, z; } V3f;

// clang-format on

f32 V2fDistance(V2f point1, V2f point2)
{
    f32 dx = point1.x - point2.x;
    f32 dy = point1.y - point2.y;
    return Sqrt(dx * dx + dy * dy);
}

inline V2f V2fMult(V2f p, f32 scalar)
{
    return (V2f){p.x * scalar, p.y * scalar};
}

inline V2f V2fAdd(V2f p1, V2f p2)
{
    return (V2f){p1.x + p2.x, p1.y + p2.y};
}

inline V2f V2fAddScalar(V2f p1, f32 scalar)
{
    return (V2f){p1.x + scalar, p1.y + scalar};
}

inline V2f V2fDiffScalar(V2f p1, f32 scalar)
{
    return (V2f){p1.x - scalar, p1.y - scalar};
}

inline V2f V2fDiff(V2f p1, V2f p2)
{
    return (V2f){p1.x - p2.x, p1.y - p2.y};
}

inline V2f V2fNormalize(V2f v)
{
    float magnitude = Sqrt(v.x * v.x + v.y * v.y);

    if (magnitude == 0.0f)
        return v;
    else
        return (V2f){v.x / magnitude, v.y / magnitude};
}

//
// Matrixes
//
//

typedef struct Mat4
{
    float values[16];
} Mat4;

inline Mat4 CreateScreenProjection(V2i screen)
{
    // allows me to set vecrtex coords as 0..width/height, instead of -1..+1
    // 0,0 is bottom left, not top left
    // matrix in code != matrix in math notation, details at https://youtu.be/kBuaCqaCYwE?t=3084
    // in short: rows in math are columns in code
    // clang-format off
    return (Mat4){
        2.0f / (f32)screen.x, 0,                    0, -1,
        0,                    2.0f / (f32)screen.y, 0, -1,
        0,                    0,                    1,  0,
        0,                    0,                    0,  1,
    };
    // clang-format on
}

inline Mat4 Mat4TranslateV3f(Mat4 mat, V3f v)
{
    mat.values[3 + 0 * 4] += v.x;
    mat.values[3 + 1 * 4] += v.y;
    mat.values[3 + 2 * 4] += v.z;
    return mat;
}

inline Mat4 Mat4TranslateV2f(Mat4 mat, V2f v)
{
    mat.values[3 + 0 * 4] += v.x;
    mat.values[3 + 1 * 4] += v.y;
    return mat;
}

inline Mat4 Mat4TranslateXY(Mat4 mat, f32 x, f32 y)
{
    mat.values[3 + 0 * 4] += x;
    mat.values[3 + 1 * 4] += y;
    return mat;
}

inline Mat4 Mat4Scale1f(Mat4 mat, float v)
{
    mat.values[0 + 0 * 4] *= v;
    mat.values[1 + 1 * 4] *= v;
    mat.values[2 + 2 * 4] *= v;
    return mat;
}

inline Mat4 Mat4ScaleV3f(Mat4 mat, V3f v)
{
    mat.values[0 + 0 * 4] *= v.x;
    mat.values[1 + 1 * 4] *= v.y;
    mat.values[2 + 2 * 4] *= v.z;
    return mat;
}

inline Mat4 Mat4ScaleUniform(Mat4 mat, f32 scalar)
{
    mat.values[0 + 0 * 4] *= scalar;
    mat.values[1 + 1 * 4] *= scalar;
    mat.values[2 + 2 * 4] *= scalar;
    return mat;
}

inline Mat4 Mat4ScaleXY(Mat4 mat, f32 x, f32 y)
{
    mat.values[0 + 0 * 4] *= x;
    mat.values[1 + 1 * 4] *= y;
    return mat;
}

inline Mat4 Mat4Identity()
{
    // clang-format off
    return (Mat4){
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };
    // clang-format on
}
