#pragma once
#include "utils\all.c"
#include <dsound.h>

#define _USE_MATH_DEFINES
#include "math.h"

typedef DirectSoundCreateType(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);

DirectSoundCreateType *soundCreate;
// extern _Check_return_ HRESULT WINAPI DirectSoundCreate(_In_opt_ LPCGUID pcGuidDevice, _Outptr_ LPDIRECTSOUND *ppDS, _Pre_null_ LPUNKNOWN pUnkOuter);

LPDIRECTSOUNDBUFFER soundBuffer;

i32 samplesPerSecond = 48000;
i32 toneHz = 256;
i16 toneVolume = 3000;

u32 runningSampleIndex = 0;
DWORD bufferSize;
f32 wavePeriod;
i32 bytesPerSample;

f32 tSine;
i32 latencySampleCount;

void InitSound(HWND window)
{
    wavePeriod = (f32)samplesPerSecond / (f32)toneHz;
    bytesPerSample = sizeof(i16) * 2;
    bufferSize = samplesPerSecond * bytesPerSample;

    latencySampleCount = samplesPerSecond / 15;

    // Load library
    HMODULE soundLib = LoadLibraryA("dsound.dll");
    if (soundLib)
    {
        soundCreate = (DirectSoundCreateType *)GetProcAddress(soundLib, "DirectSoundCreate");

        LPDIRECTSOUND sound;
        if (soundCreate && SUCCEEDED(soundCreate(0, &sound, 0)))
        {
            WAVEFORMATEX WaveFormat = {0};
            WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
            WaveFormat.nChannels = 2;
            WaveFormat.nSamplesPerSec = samplesPerSecond;
            WaveFormat.wBitsPerSample = 16;
            WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
            WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
            WaveFormat.cbSize = 0;

            if (SUCCEEDED(sound->lpVtbl->SetCooperativeLevel(sound, window, DSSCL_PRIORITY)))
            {
                DSBUFFERDESC BufferDescription = {0};
                BufferDescription.dwSize = sizeof(BufferDescription);
                BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

                // NOTE(casey): "Create" a primary buffer
                // TODO(casey): DSBCAPS_GLOBALFOCUS?
                LPDIRECTSOUNDBUFFER PrimaryBuffer;
                if (SUCCEEDED(sound->lpVtbl->CreateSoundBuffer(sound, &BufferDescription, &PrimaryBuffer, 0)))
                {
                    HRESULT Error = PrimaryBuffer->lpVtbl->SetFormat(PrimaryBuffer, &WaveFormat);
                    if (SUCCEEDED(Error))
                    {
                        // NOTE(casey): We have finally set the format!
                        OutputDebugStringA("Primary buffer format was set.\n");
                    }
                }
            }

            DSBUFFERDESC BufferDescription = {0};
            BufferDescription.dwSize = sizeof(BufferDescription);
            BufferDescription.dwFlags = 0;
            BufferDescription.dwBufferBytes = bufferSize;
            BufferDescription.lpwfxFormat = &WaveFormat;

            HRESULT Error = sound->lpVtbl->CreateSoundBuffer(sound, &BufferDescription, &soundBuffer, 0);
            if (SUCCEEDED(Error))
            {
                OutputDebugStringA("Secondary buffer created successfully.\n");
            }
        }
    }
}

void FillSoundRegion(void *region, DWORD size)
{
    DWORD Region1SampleCount = size / bytesPerSample;
    i16 *SampleOut = (i16 *)region;
    for (DWORD i = 0; i < Region1SampleCount; ++i)
    {
        f32 SineValue = sinf(tSine);
        // f32 CosValue;
        // SinCos(tSine, &SineValue, &CosValue);
        i16 SampleValue = (i16)(SineValue * toneVolume);
        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;

        soundSamples[currentPosition++] = SampleValue;

        if (currentPosition >= SAMPLES_TO_SHOW)
            currentPosition = 0;

        tSine += 2.0f * M_PI * 1.0f / wavePeriod;
        ++runningSampleIndex;
    }
}

void FillSoundBuffer(DWORD ByteToLock, DWORD BytesToWrite)
{
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;
    if (SUCCEEDED(soundBuffer->lpVtbl->Lock(soundBuffer, ByteToLock, BytesToWrite,
                                            &Region1, &Region1Size,
                                            &Region2, &Region2Size,
                                            0)))
    {
        FillSoundRegion(Region1, Region1Size);
        FillSoundRegion(Region2, Region2Size);

        soundBuffer->lpVtbl->Unlock(soundBuffer, Region1, Region1Size, Region2, Region2Size);
    }
}

void FirstFillSoundAndPlay()
{
    FillSoundBuffer(0, latencySampleCount * bytesPerSample);

    soundBuffer->lpVtbl->Play(soundBuffer, 0, 0, DSBPLAY_LOOPING);
}

void WriteSound()
{
    // NOTE(casey): DirectSound output test
    DWORD PlayCursor;
    DWORD WriteCursor;
    if (SUCCEEDED(soundBuffer->lpVtbl->GetCurrentPosition(soundBuffer, &PlayCursor, &WriteCursor)))
    {
        DWORD ByteToLock = ((runningSampleIndex * bytesPerSample) % bufferSize);

        DWORD TargetCursor = ((PlayCursor + (latencySampleCount * bytesPerSample)) % bufferSize);
        DWORD BytesToWrite;
        // TODO(casey): Change this to using a lower latency offset from the playcursor
        // when we actually start having sound effects.
        if (ByteToLock > TargetCursor)
        {
            BytesToWrite = (bufferSize - ByteToLock);
            BytesToWrite += TargetCursor;
        }
        else
        {
            BytesToWrite = TargetCursor - ByteToLock;
        }

        FillSoundBuffer(ByteToLock, BytesToWrite);
    }
}