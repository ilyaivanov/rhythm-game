#include <windows.h>
// #include <stdio.h>
// #include <math.h>
#include <gl/gl.h>

#include "types.h"
#include "win32.c"
#include "sincos.c"
#include "performance.c"
#include "..\format.c"

#define ONE_OVER_SQUARE_ROOT_OF_TWO 0.70710678118f

V2i clientAreaSize = {0};
i32 isRunning = 1;
i32 isFullscreen = 1;
f32 appTime = 0;
Mat4 projection;

V2f pos = {0};
f32 speed = 40;
f32 size = 80;

char keys[256] = {0};

inline Mat4 CreateScreenProjection(V2i screen)
{
    // allows me to set vecrtex coords as 0..width/height, instead of -1..+1
    // 0,0 is bottom left, not top left
    // matrix in code != matrix in math notation, details at https://youtu.be/kBuaCqaCYwE?t=3084
    // in short: rows in math are columns in code
    return (Mat4){
        2.0f / (f32)screen.x,
        0,
        0,
        -1,
        0,
        2.0f / (f32)screen.y,
        0,
        -1,
        0,
        0,
        1,
        0,
        0,
        0,
        0,
        1,
    };
}

void OnResize()
{
    glViewport(0, 0, clientAreaSize.x, clientAreaSize.y);
    projection = CreateScreenProjection(clientAreaSize);
}

LRESULT OnEvent(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_DESTROY)
    {
        isRunning = 0;
    }
    else if (message == WM_SIZE)
    {
        clientAreaSize.x = LOWORD(lParam);
        clientAreaSize.y = HIWORD(lParam);
        OnResize();
        InvalidateRect(window, NULL, TRUE);
    }
    else if (message == WM_PAINT)
    {
        PAINTSTRUCT paint = {0};
        HDC dc = BeginPaint(window, &paint);
        // Draw();
        EndPaint(window, &paint);
        SwapBuffers(dc);
    }
    else if (message == WM_KEYDOWN)
    {
        if (wParam < 256)
            keys[wParam] = 1;
        if (wParam == VK_SPACE)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        if (wParam == VK_F11)
        {
            isFullscreen = !isFullscreen;
            SetFullscreen(window, isFullscreen);
        }
    }
    else if (message == WM_KEYUP)
    {
        if (wParam < 256)
            keys[wParam] = 0;
        if (wParam == VK_SPACE)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    return DefWindowProc(window, message, wParam, lParam);
}

GLuint uiProgram;
GLuint vertexBuffer;
GLuint vertexArray;

#define POINTS_PER_VERTEX 2
float vertices[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f};

inline Mat4 Mat4TranslateV3f(Mat4 mat, V3f v)
{
    mat.values[3 + 0 * 4] += v.x;
    mat.values[3 + 1 * 4] += v.y;
    mat.values[3 + 2 * 4] += v.z;
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

inline Mat4 Mat4Identity()
{
    return (Mat4){
        1,
        0,
        0,
        0,
        0,
        1,
        0,
        0,
        0,
        0,
        1,
        0,
        0,
        0,
        0,
        1,
    };
}

void __stdcall WinMainCRTStartup()
{
    InitPerf();

    HINSTANCE instance = GetModuleHandle(0);
    PreventWindowsDPIScaling();
    HWND window = OpenAppWindowWithSize(instance, OnEvent, 1800, 1000);
    HDC dc = GetDC(window);

    Win32InitOpenGL(window);
    OnResize();
    InitFunctions();

    if (isFullscreen)
        SetFullscreen(window, isFullscreen);

    uiProgram = CreateProgram("..\\glExample\\primitives.vs", "..\\glExample\\primitives.fs");

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //
    //
    //
    //

    glGenVertexArrays(1, &vertexArray);

    glBindVertexArray(vertexArray);
    size_t stride = POINTS_PER_VERTEX * sizeof(float);
    glVertexAttribPointer(0, POINTS_PER_VERTEX, GL_FLOAT, GL_FALSE, stride, (void *)0);
    glEnableVertexAttribArray(0);

    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void *)(3 * sizeof(float)));
    // glEnableVertexAttribArray(1);

    // glEnable(GL_DEPTH_TEST);

    // Mat4 view = CreateScreenProjection(clientAreaSize);
    glUseProgram(uiProgram);

    while (isRunning)
    {
        StartMetric(Overall);
        MSG msg;
        while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        V2f delta = {0};
        if (keys['W'])
            delta.y = 1.0f;
        else if (keys['S'])
            delta.y = -1.0f;

        if (keys['A'])
            delta.x = -1.0f;
        else if (keys['D'])
            delta.x = 1.0f;

        if (delta.x != 0.0f && delta.y != 0.0f)
            delta = V2fMult(delta, ONE_OVER_SQUARE_ROOT_OF_TWO);

        delta = V2fMult(delta, speed);

        // Draw();
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // f32 sin;
        // f32 cos;
        // SinCos(appTime / 1000, &sin, &cos);
        // pos.values[0] = (cos + 1.0f) / 2.0f;

        pos = V2fAdd(pos, delta);
        V3f basePosition = {pos.x, pos.y, 0.0f};

        char buff[40];
        FormatNumber(GetMicrosecondsFor(Overall), buff);
        OutputDebugStringA(buff);
        OutputDebugStringA("\n");

        Mat4 model = Mat4ScaleV3f(Mat4TranslateV3f(Mat4Identity(), basePosition), (V3f){size, size, 0});
        glUniformMatrix4fv(glGetUniformLocation(uiProgram, "projection"), 1, GL_TRUE, projection.values);
        glUniformMatrix4fv(glGetUniformLocation(uiProgram, "view"), 1, GL_TRUE, model.values);

        glBindVertexArray(vertexArray);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, ArrayLength(vertices) / POINTS_PER_VERTEX);

        SwapBuffers(dc);

        appTime += 16.66666f;
        EndMetric(Overall);
    }
    ExitProcess(0);
}