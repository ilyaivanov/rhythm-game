#include <windows.h>
#include <windowsx.h> // this is used for GET_X_PARAM and GET_Y_PARAM
// #include <stdio.h>
// #include <math.h>
#include <gl/gl.h>

#include "utils/all.c"
#include "win32.c"
#include "constants.c"
#include "bullets.c"
#include "enemies.c"

#define ONE_OVER_SQUARE_ROOT_OF_TWO 0.70710678118f

V2i clientAreaSize = {0};
i32 isRunning = 1;
i32 isFullscreen = 1;
f32 appTime = 0;
Mat4 projection;

V2f pos = {0};

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
    else if (message == WM_LBUTTONDOWN)
    {
        V2f mouse = {GET_X_LPARAM(lParam), clientAreaSize.y - GET_Y_LPARAM(lParam)};
        Fire(V2fAddScalar(pos, playerSize / 2), mouse);
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
    InitGlFunctions();

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
    GLint colorLocation = glGetUniformLocation(uiProgram, "color");

    InitEnemies();
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

        // Update
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

        delta = V2fMult(delta, playerSpeed);
        pos = V2fAdd(pos, delta);

        pos.x = Clamp(pos.x, 0, clientAreaSize.x - playerSize);
        pos.y = Clamp(pos.y, 0, clientAreaSize.y - playerSize);

        UpdateEnemies(clientAreaSize);
        UpdateBullets(delta);

        // Draw
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUniformMatrix4fv(projectionLocation, 1, GL_TRUE, projection.values);
        glUniform4f(colorLocation, 1.0f, 1.0f, 1.0f, 1.0f);

        Mat4 view = Mat4ScaleUniform(Mat4TranslateV2f(Mat4Identity(), pos), playerSize);
        glUniformMatrix4fv(viewLocation, 1, GL_TRUE, view.values);

        glBindVertexArray(vertexArray);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, ArrayLength(vertices) / POINTS_PER_VERTEX);

        DrawBullets(viewLocation, colorLocation);
        DrawEnemies(viewLocation, colorLocation);

        EndMetric(OverallWithoutSwap);
        SwapBuffers(dc);

        appTime += 16.66666f;
        EndMetric(Overall);
    }
    ExitProcess(0);
}