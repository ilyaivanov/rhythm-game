#include <windows.h>
// #include <stdio.h>
// #include <math.h>
#include <gl/gl.h>

#include "utils/all.c"
#include "win32.c"

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
float vertices[] = {0.0f, 0.0f,
                    1.0f, 0.0f,
                    0.0f, 1.0f,
                    1.0f, 1.0f};

void __stdcall WinMainCRTStartup()
{
    InitPerf();

    HINSTANCE instance = GetModuleHandle(0);
    PreventWindowsDPIScaling();
    HWND window = OpenAppWindowWithSize(instance, OnEvent, 1800, 1000);
    HDC dc = GetDC(window);

    Win32InitOpenGL(window, dc);
    OnResize();
    InitFunctions();

    if (isFullscreen)
        SetFullscreen(window, isFullscreen);

    uiProgram = CreateProgram("..\\shaders\\primitives.vs", "..\\shaders\\primitives.fs");

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vertexArray);

    glBindVertexArray(vertexArray);
    size_t stride = POINTS_PER_VERTEX * sizeof(float);
    glVertexAttribPointer(0, POINTS_PER_VERTEX, GL_FLOAT, GL_FALSE, stride, (void *)0);
    glEnableVertexAttribArray(0);

    // glEnable(GL_DEPTH_TEST);

    glUseProgram(uiProgram);

    GLint projectionLocation = glGetUniformLocation(uiProgram, "projection");
    GLint viewLocation = glGetUniformLocation(uiProgram, "view");
    while (isRunning)
    {
        StartMetric(Overall);
        StartMetric(OverallWithoutSwap);
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

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        pos = V2fAdd(pos, delta);
        V3f basePosition = {pos.x, pos.y, 0.0f};

        Mat4 model = Mat4ScaleV3f(Mat4TranslateV3f(Mat4Identity(), basePosition), (V3f){size, size, 0});
        glUniformMatrix4fv(projectionLocation, 1, GL_TRUE, projection.values);
        glUniformMatrix4fv(viewLocation, 1, GL_TRUE, model.values);

        glBindVertexArray(vertexArray);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, ArrayLength(vertices) / POINTS_PER_VERTEX);

        EndMetric(OverallWithoutSwap);
        SwapBuffers(dc);

        appTime += 16.66666f;
        EndMetric(Overall);
    }
    ExitProcess(0);
}