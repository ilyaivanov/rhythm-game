#include <windows.h>
// #include <stdio.h>
// #include <math.h>
#include <gl/gl.h>

#include "win32.c"
#include "sincos.c"

V2i clientAreaSize = {0};
i32 isRunning = 1;
i32 isFullscreen = 0;
f32 appTime = 0;

void OnResize()
{
    glViewport(0, 0, clientAreaSize.x, clientAreaSize.y);
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
    -0.5f, -0.5f,
    0.5f, -0.5f,
    0.0f, 0.5f};

void __stdcall WinMainCRTStartup()
{
    HINSTANCE instance = GetModuleHandle(0);
    PreventWindowsDPIScaling();
    HWND window = OpenAppWindowWithSize(instance, OnEvent, 1800, 1800);
    HDC dc = GetDC(window);

    Win32InitOpenGL(window);
    OnResize();
    InitFunctions();

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

    Mat4 pos = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    glUseProgram(uiProgram);

    while (isRunning)
    {
        MSG msg;
        while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // Draw();
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        f32 sin;
        f32 cos;
        SinCos(appTime / 1000, &sin, &cos);
        pos.values[0] = (cos + 1.0f) / 2.0f;

        glUniformMatrix4fv(glGetUniformLocation(uiProgram, "projection"), 1, GL_TRUE, pos.values);

        glBindVertexArray(vertexArray);
        glDrawArrays(GL_TRIANGLES, 0, ArrayLength(vertices) / POINTS_PER_VERTEX);

        SwapBuffers(dc);

        appTime += 16.66666f;
    }
    ExitProcess(0);
}