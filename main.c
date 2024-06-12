#include <windows.h>
#include <windowsx.h> // this is used for GET_X_PARAM and GET_Y_PARAM
// #include <stdio.h>
// #include <math.h>
#include <gl/gl.h>

#include "win32.c"
#include "utils/all.c"
#include "utils/font.c"
#include "constants.c"
#include "bullets.c"
#include "enemies.c"
#include "particles.c"

#define ONE_OVER_SQUARE_ROOT_OF_TWO 0.70710678118f

V2i clientAreaSize = {0};
i32 isRunning = 1;
i32 isFullscreen = 0;
f32 appTime = 0;
Mat4 projection;

V2f playerPosition = {0};

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
        Fire(V2fAddScalar(playerPosition, playerSize / 2), mouse, PlayerBullet, playerBulletSpeed, playerBulletSize);
    }

    return DefWindowProc(window, message, wParam, lParam);
}

GLuint uiProgram;
GLuint vertexBuffer;
GLuint vertexArray;

GLuint fontVertexBuffer;
GLuint fontVertexArray;
GLuint fontProgram;

#define FLOATS_PER_VERTEX 4
float fontVertices[] = {
    // Position      UV coords
    1.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 1.0f};

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
    InitFonts();

    if (isFullscreen)
        SetFullscreen(window, isFullscreen);

    uiProgram = CreateProgram("..\\shaders\\primitives.vs", "..\\shaders\\primitives.fs");
    fontProgram = CreateProgram("..\\shaders\\font.vs", "..\\shaders\\font.fs");

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vertexArray);

    glBindVertexArray(vertexArray);
    size_t stride = POINTS_PER_VERTEX * sizeof(float);
    glVertexAttribPointer(0, POINTS_PER_VERTEX, GL_FLOAT, GL_FALSE, stride, (void *)0);
    glEnableVertexAttribArray(0);

    //
    // Font
    //
    glGenBuffers(1, &fontVertexBuffer);
    glGenVertexArrays(1, &fontVertexArray);

    glBindBuffer(GL_ARRAY_BUFFER, fontVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fontVertices), fontVertices, GL_STATIC_DRAW);

    glBindVertexArray(fontVertexArray);
    size_t strideFont = FLOATS_PER_VERTEX * sizeof(float);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, strideFont, (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, strideFont, (void *)0);
    glEnableVertexAttribArray(1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLint projectionLocation = glGetUniformLocation(uiProgram, "projection");
    GLint viewLocation = glGetUniformLocation(uiProgram, "view");
    GLint colorLocation = glGetUniformLocation(uiProgram, "color");

    GLint projectionLocationFont = glGetUniformLocation(fontProgram, "projection");
    GLint viewLocationFont = glGetUniformLocation(fontProgram, "view");

    InitEnemies();
    InitParticles();
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
        playerPosition = V2fAdd(playerPosition, delta);

        playerPosition.x = Clamp(playerPosition.x, 0, clientAreaSize.x - playerSize);
        playerPosition.y = Clamp(playerPosition.y, 0, clientAreaSize.y - playerSize);

        HandleCollisions(playerPosition, clientAreaSize);

        UpdateBullets(playerPosition, clientAreaSize);
        UpdateEnemies(clientAreaSize, playerPosition);
        UpdateParticles();

        // Draw
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(uiProgram);
        glUniformMatrix4fv(projectionLocation, 1, GL_TRUE, projection.values);
        glUniform4f(colorLocation, 1.0f, 1.0f, 1.0f, 1.0f);

        Mat4 view = Mat4ScaleUniform(Mat4TranslateV2f(Mat4Identity(), playerPosition), playerSize);
        glUniformMatrix4fv(viewLocation, 1, GL_TRUE, view.values);

        glBindVertexArray(vertexArray);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, ArrayLength(vertices) / POINTS_PER_VERTEX);

        DrawParticles(viewLocation, colorLocation);
        DrawEnemies(viewLocation, colorLocation, (V2f){(f32)clientAreaSize.x, (f32)clientAreaSize.y});
        DrawBullets(viewLocation, colorLocation);

        //
        // Font stuff
        //
        glUseProgram(fontProgram);
        glBindVertexArray(fontVertexArray);
        glUniformMatrix4fv(projectionLocationFont, 1, GL_TRUE, projection.values);
        float charWidth = codeFont.textures['W'].width;
        float charHeight = codeFont.textures['W'].height;
        char ms[200];
        i32 len = FormatNumber(GetMicrosecondsFor(Overall), ms);
        for (i32 i = 0; i < len; i++)
        {

            Mat4 view2 = Mat4ScaleXY(Mat4TranslateV2f(Mat4Identity(), (V2f){clientAreaSize.x - (i * charWidth), 0}), charWidth, charHeight);
            glUniformMatrix4fv(viewLocationFont, 1, GL_TRUE, view2.values);

            glBindTexture(GL_TEXTURE_2D, codeFont.openglTextureIds[ms[i]]);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, ArrayLength(fontVertices) / FLOATS_PER_VERTEX);
        }

        EndMetric(OverallWithoutSwap);
        SwapBuffers(dc);

        appTime += 16.66666f;
        EndMetric(Overall);
    }
    ExitProcess(0);
}