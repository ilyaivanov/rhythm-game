#pragma once
#include <windows.h>
#include "types.h"

typedef enum PerfMetric
{
    Overall,
    OverallWithoutSwap,
    Memory,
    Draw,
    DiBits,
    TotalMetrics
} PerfMetric;

u64 frequency;
u64 starts[TotalMetrics];
// u64 ends[TotalMetrics];
u32 perfsMicroseconds[TotalMetrics];

u32 GetMicrosecondsFor(PerfMetric metric)
{
    return perfsMicroseconds[metric];
}

inline u64 GetPerfCounter()
{
    LARGE_INTEGER end;
    QueryPerformanceCounter(&end);
    return end.QuadPart;
}

void StartMetric(PerfMetric metric)
{
    starts[metric] = GetPerfCounter();
}

void EndMetric(PerfMetric metric)
{
    perfsMicroseconds[metric] = (u32)((f32)((GetPerfCounter() - starts[metric]) * 1000 * 1000) / (f32)frequency);
}

void InitPerf()
{
    LARGE_INTEGER fre = {0};
    QueryPerformanceFrequency(&fre);
    frequency = fre.QuadPart;
    StartMetric(Overall);
}
