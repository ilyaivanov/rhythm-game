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

void StartMetric(PerfMetric metric)
{
    LARGE_INTEGER start = {0};
    QueryPerformanceCounter(&start);
    starts[metric] = start.QuadPart;
}

void EndMetric(PerfMetric metric)
{
    LARGE_INTEGER end = {0};
    QueryPerformanceCounter(&end);
    // ends[metric] = end.QuadPart;
    perfsMicroseconds[metric] = (u32)((f32)((end.QuadPart - starts[metric]) * 1000 * 1000) / (f32)frequency);
    // return GetMicrosecondsFor(metric);
}

void InitPerf()
{
    LARGE_INTEGER fre = {0};
    QueryPerformanceFrequency(&fre);
    frequency = fre.QuadPart;
    StartMetric(Overall);
}
