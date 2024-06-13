#pragma once

#include <windows.h>

// used to set dark mode
#include <dwmapi.h>
#include "utils/types.h"

#include "gl/glFlags.h"
#include "gl/glFunctions.c"

// int _fltused = 0x9875;
#define VirtualAllocateMemory(size) (VirtualAlloc(0, size, MEM_COMMIT, PAGE_READWRITE))
#define VirtualFreeMemory(ptr) (VirtualFree(ptr, 0, MEM_RELEASE))

void Win32InitOpenGL(HWND Window, HDC dc)
{
    PIXELFORMATDESCRIPTOR DesiredPixelFormat = {0};
    DesiredPixelFormat.nSize = sizeof(DesiredPixelFormat);
    DesiredPixelFormat.nVersion = 1;
    DesiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
    DesiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    DesiredPixelFormat.cColorBits = 32;
    DesiredPixelFormat.cAlphaBits = 8;
    DesiredPixelFormat.iLayerType = PFD_MAIN_PLANE;

    int SuggestedPixelFormatIndex = ChoosePixelFormat(dc, &DesiredPixelFormat);
    PIXELFORMATDESCRIPTOR SuggestedPixelFormat;
    DescribePixelFormat(dc, SuggestedPixelFormatIndex,
                        sizeof(SuggestedPixelFormat), &SuggestedPixelFormat);
    SetPixelFormat(dc, SuggestedPixelFormatIndex, &SuggestedPixelFormat);

    HGLRC OpenGLRC = wglCreateContext(dc);
    if (!wglMakeCurrent(dc, OpenGLRC))
        Fail("Failed to initialize OpenGL");
}

HWND OpenAppWindowWithSize(HINSTANCE instance, WNDPROC OnEvent, int windowWidth, int windowHeight)
{
    WNDCLASSW windowClass = {0};
    windowClass.hInstance = instance;
    windowClass.lpfnWndProc = OnEvent;
    windowClass.lpszClassName = L"MyWindow";
    windowClass.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
    windowClass.hCursor = LoadCursor(0, IDC_ARROW);
    // not using COLOR_WINDOW + 1 because it doesn't fucking work
    // this line fixes a flash of a white background for 1-2 frames during start
    windowClass.hbrBackground = CreateSolidBrush(0x111111);
    RegisterClassW(&windowClass);

    HDC dc = GetDC(0);
    int screenWidth = GetDeviceCaps(dc, HORZRES);
    int screenHeight = GetDeviceCaps(dc, VERTRES);

    HWND window = CreateWindowW(windowClass.lpszClassName, (wchar_t *)"Editor", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                /* x */ screenWidth - windowWidth - 20,
                                /* y */ 0 + 20,
                                /* w */ windowWidth,
                                /* h */ windowHeight,
                                0, 0, instance, 0);

    BOOL USE_DARK_MODE = TRUE;
    BOOL SET_IMMERSIVE_DARK_MODE_SUCCESS = SUCCEEDED(DwmSetWindowAttribute(
        window, DWMWA_USE_IMMERSIVE_DARK_MODE, &USE_DARK_MODE, sizeof(USE_DARK_MODE)));

    return window;
}

// Increasing Read Bandwidth with SIMD Instructions https://www.computerenhance.com/p/increasing-read-bandwidth-with-simd
// #pragma function(memset)
// void *memset(void *dest, int c, size_t count)
// {
//     char *bytes = (char *)dest;
//     while (count--)
//     {
//         *bytes++ = (char)c;
//     }
//     return dest;
// }

// DPI Scaling
// user32.dll is linked statically, so dynamic linking won't load that dll again
// taken from https://github.com/cmuratori/refterm/blob/main/refterm.c#L80
// this is done because GDI font drawing is ugly and unclear when DPI scaling is enabled

typedef BOOL WINAPI set_process_dpi_aware(void);
typedef BOOL WINAPI set_process_dpi_awareness_context(DPI_AWARENESS_CONTEXT);
static void PreventWindowsDPIScaling()
{
    HMODULE WinUser = LoadLibraryW(L"user32.dll");
    set_process_dpi_awareness_context *SetProcessDPIAwarenessContext = (set_process_dpi_awareness_context *)GetProcAddress(WinUser, "SetProcessDPIAwarenessContext");
    if (SetProcessDPIAwarenessContext)
    {
        SetProcessDPIAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
    }
    else
    {
        set_process_dpi_aware *SetProcessDPIAware = (set_process_dpi_aware *)GetProcAddress(WinUser, "SetProcessDPIAware");
        if (SetProcessDPIAware)
        {
            SetProcessDPIAware();
        }
    }
}

// https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353
WINDOWPLACEMENT prevWindowDimensions = {sizeof(prevWindowDimensions)};
void SetFullscreen(HWND window, i32 isFullscreen)
{
    DWORD style = GetWindowLong(window, GWL_STYLE);
    if (isFullscreen)
    {
        MONITORINFO monitorInfo = {sizeof(monitorInfo)};
        if (GetWindowPlacement(window, &prevWindowDimensions) &&
            GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &monitorInfo))
        {
            SetWindowLong(window, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);

            SetWindowPos(window, HWND_TOP,
                         monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
                         monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                         monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowLong(window, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(window, &prevWindowDimensions);
        SetWindowPos(window, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

GLuint CompileShader(GLuint shaderEnum, const char *source)
{
    GLuint shader = glCreateShader(shaderEnum);
    glShaderSource(shader, 1, &source, NULL);

    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    char *shaderName = shaderEnum == GL_VERTEX_SHADER ? "Vertex" : "Fragmment";
    if (success)
    {
        OutputDebugStringA(shaderName);
        OutputDebugStringA("Shader Compiled\n");
    }
    else
    {
        OutputDebugStringA(shaderName);
        OutputDebugStringA("Shader Errors\n");

        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        OutputDebugStringA(infoLog);
        OutputDebugStringA("\n");
    }
    return shader;
}

typedef struct FileContent
{
    char *content;
    i32 size;
} FileContent;

FileContent ReadMyFileImp(char *path)
{
    HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

    LARGE_INTEGER size;
    GetFileSizeEx(file, &size);

    u32 fileSize = (u32)size.QuadPart;

    void *buffer = VirtualAllocateMemory(fileSize);

    DWORD bytesRead;
    ReadFile(file, buffer, fileSize, &bytesRead, 0);
    CloseHandle(file);

    FileContent res = {0};
    res.content = (char *)buffer;
    res.size = bytesRead;
    return res;
}

GLuint CreateProgram(char *vertexShaderPath, char *fragmentShaderPath)
{
    FileContent vertexFile = ReadMyFileImp(vertexShaderPath);
    FileContent fragmentFile = ReadMyFileImp(fragmentShaderPath);

    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexFile.content);
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentFile.content);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);
    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success)
        OutputDebugStringA("Program Linked\n");
    else
    {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        OutputDebugStringA("Error during linking: \n");
        OutputDebugStringA(infoLog);
        OutputDebugStringA("\n");
    }

    // TODO: there is no error checking, just learning stuff, not writing prod code
    VirtualFreeMemory(fragmentFile.content);
    VirtualFreeMemory(vertexFile.content);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}