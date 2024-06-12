#pragma once
#include "types.h"
#include <xmmintrin.h>
#include <intrin.h>

// taken from 457 day codebase of handmade hero

#define U32Max ((u32) - 1)
typedef struct f32_4x
{
    union
    {
        __m128 P;
        f32 E[4];
        u32 U32[4];
    };
} f32_4x;

typedef struct RandomSeries
{
    f32_4x State;
} RandomSeries;

inline f32_4x F32_4x(__m128 EAll)
{
    f32_4x Result;

    Result.P = EAll;

    return Result;
}

inline f32_4x U32_4x(u32 E0, u32 E1, u32 E2, u32 E3)
{
    f32_4x Result;

    Result.P = _mm_setr_ps(*(float *)&E0, *(float *)&E1, *(float *)&E2, *(float *)&E3);

    return Result;
}

inline f32_4x f32_4xXOR(f32_4x A, f32_4x B)
{
    f32_4x Result;

    Result.P = _mm_xor_ps(A.P, B.P);

    return Result;
}

#define ShiftRight4X(A, Imm) F32_4x(_mm_castsi128_ps(_mm_srli_epi32(_mm_castps_si128(A.P), Imm)))
#define ShiftLeft4X(A, Imm) F32_4x(_mm_castsi128_ps(_mm_slli_epi32(_mm_castps_si128(A.P), Imm)))

inline RandomSeries CreateSeriesSeeded(u32 e0, u32 e1, u32 e2, u32 e3)
{
    RandomSeries series;
    series.State = U32_4x(e0, e1, e2, e3);
    return series;
}

inline RandomSeries CreateSeries()
{
    return CreateSeriesSeeded(78953890, 235498, 893456, 93453080);
}

inline f32_4x RandomNextU324X(RandomSeries *Series)
{
    f32_4x Result = Series->State;
    Result = f32_4xXOR(Result, ShiftLeft4X(Result, 13));
    Result = f32_4xXOR(Result, ShiftRight4X(Result, 17));
    Result = f32_4xXOR(Result, ShiftLeft4X(Result, 5));
    Series->State = Result;

    return Result;
}

inline u32 RandomNextU32(RandomSeries *Series)
{
    return RandomNextU324X(Series).U32[0];
}

inline u32 RandomChoice(RandomSeries *Series, u32 ChoiceCount)
{
    return RandomNextU32(Series) % ChoiceCount;
}

inline f32 RandomF32Normal(RandomSeries *series)
{
    f32 Divisor = 1.0f / (f32)U32Max;
    return Divisor * (f32)RandomNextU32(series);
}

inline f32 RandomF32(RandomSeries *series, f32 min, f32 max)
{
    return min + RandomF32Normal(series) * (max - min);
}

inline V2f RandomUnitVector(RandomSeries *series)
{
    float rad = RandomF32(series, 0.0f, 2 * E_PI);
    V2f res;
    SinCos(rad, &res.y, &res.x);
    return res;
}