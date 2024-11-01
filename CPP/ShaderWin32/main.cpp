#include <windows.h>
#include <math.h>
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <gl/GL.h>

static HGLRC hRC; // Rendering Context
static HDC hDC;   // Device Context
static HWND hWnd; // Window Handle

static UINT32 shaderProgram;
static INT32 timeLocation;
static FLOAT time = 0.0f;

typedef uint32_t (*GLCreateProgramFunc)();
GLCreateProgramFunc glCreateProgram = NULL;
typedef uint32_t (*GLCreateShaderFunc)(uint32_t);
GLCreateShaderFunc glCreateShader = NULL;
typedef int (*GLGetUniformLocationFunc)(uint32_t, const char*);
GLGetUniformLocationFunc glGetUniformLocation = NULL;
typedef void (*GLShaderSourceFunc)(uint32_t, int, const char**, const int*);
GLShaderSourceFunc glShaderSource = NULL;
typedef void (*GLCompileShaderFunc)(uint32_t);
GLCompileShaderFunc glCompileShader = NULL;
typedef void (*GLLinkProgramFunc)(uint32_t);
GLLinkProgramFunc glLinkProgram = NULL;
typedef void (*GLUseProgramFunc)(uint32_t);
GLUseProgramFunc glUseProgram = NULL;
typedef void (*GLAttachShaderFunc)(uint32_t, uint32_t);
GLAttachShaderFunc glAttachShader = NULL;
typedef void (*GLUniform1fFunc)(int, float);
GLUniform1fFunc glUniform1f = NULL;

#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_QUADS 0x0007

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

char* vertexShaderSource = NULL;
char* fragmentShaderSource = NULL;

void InitOpenGL(HWND hwnd) {

    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32, // Color depth
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24, // Depth buffer
        8,  // Stencil buffer
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    hDC = GetDC(hwnd);
    int pixelFormat = ChoosePixelFormat(hDC, &pfd);
    SetPixelFormat(hDC, pixelFormat, &pfd);

    hRC = wglCreateContext(hDC);
    wglMakeCurrent(hDC, hRC);

    glCreateProgram = (GLCreateProgramFunc)wglGetProcAddress("glCreateProgram");
    glCreateShader = (GLCreateShaderFunc)wglGetProcAddress("glCreateShader");
    glGetUniformLocation = (GLGetUniformLocationFunc)wglGetProcAddress("glGetUniformLocation");
    glShaderSource = (GLShaderSourceFunc)wglGetProcAddress("glShaderSource");
    glCompileShader = (GLCompileShaderFunc)wglGetProcAddress("glCompileShader");
    glLinkProgram = (GLLinkProgramFunc)wglGetProcAddress("glLinkProgram");
    glUseProgram = (GLUseProgramFunc)wglGetProcAddress("glUseProgram");
    glAttachShader = (GLAttachShaderFunc)wglGetProcAddress("glAttachShader");
    glUniform1f = (GLUniform1fFunc)wglGetProcAddress("glUniform1f");

    // OpenGL initialization
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Shader setup
    shaderProgram = glCreateProgram();

    UINT32 vertexShader = glCreateShader(GL_VERTEX_SHADER);
    UINT32 fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertexShader, 1, (const char **)&vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    if(glGetError() != 0) {
        printf("Failed to compile vertex shader\n");
        exit(1);
    }
    glAttachShader(shaderProgram, vertexShader);
    glShaderSource(fragmentShader, 1, (const char**)&fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    if(glGetError() != 0) {
        printf("Failed to compile fragment shader\n");
        exit(1);
    }
    glAttachShader(shaderProgram, fragmentShader);

    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    timeLocation = glGetUniformLocation(shaderProgram, "time");
}

void RenderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glBegin(GL_QUADS);
    glVertex2f(-0.5f, -0.5f);
    glVertex2f(0.5f, -0.5f);
    glVertex2f(0.5f, 0.5f);
    glVertex2f(-0.5f, 0.5f);
    glEnd();
    SwapBuffers(hDC);
}

void Cleanup() {
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hWnd, hDC);
}

int main(int argc, char** argv) {
    char exePath[256];
    strcpy(exePath, argv[0]);
    char* lastSlash = strrchr(exePath, '\\');
    lastSlash[1] = 0;
    char vertexShaderPath[256];
    char fragmentShaderPath[256];
    strcpy(vertexShaderPath, exePath);
    strcat(vertexShaderPath, "vertex.glsl");
    strcpy(fragmentShaderPath, exePath);
    strcat(fragmentShaderPath, "frag.glsl");
    FILE *fp = 0;
    // Load vertex shader
    fp = fopen(vertexShaderPath, "rb");
    if(fp == NULL) {
        printf("Failed to open vertex shader file at %s\n", vertexShaderPath);
        return 1;
    }
    fseek(fp, 0, SEEK_END);
    UINT64 vertex_source_size = ftell(fp) + 1;
    fseek(fp, 0, SEEK_SET);
    vertexShaderSource = (char*)malloc(vertex_source_size);
    memset(vertexShaderSource, 0, vertex_source_size);
    fread(vertexShaderSource, 1, vertex_source_size, fp);
    fclose(fp);
    printf("Vertex shader source:\n%s\n############\n", vertexShaderSource);
    // Load fragment shader
    fp = fopen(fragmentShaderPath, "rb");
    if(fp == NULL) {
        printf("Failed to open fragment shader file at %s\n", fragmentShaderPath);
        return 1;
    }
    fseek(fp, 0, SEEK_END);
    UINT64 fragment_source_size = ftell(fp) + 1;
    fseek(fp, 0, SEEK_SET);
    fragmentShaderSource = (char*)malloc(fragment_source_size);
    memset(fragmentShaderSource, 0, fragment_source_size);
    fread(fragmentShaderSource, 1, fragment_source_size, fp);
    fclose(fp);
    printf("Fragment shader source:\n%s\n############\n", fragmentShaderSource);

    WNDCLASS wc = { 0 };
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"OpenGLWindowClass";

    RegisterClass(&wc);

    hWnd = CreateWindowA(
        "OpenGLWindowClass", "OpenGL Shader Animation",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        100, 100, 800, 600,
        NULL, NULL, NULL, NULL);

    InitOpenGL(hWnd);

    MSG msg;
    while (1) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            time += 0.001f;
            glUniform1f(timeLocation, time);
            RenderScene();
        }
    }

    Cleanup();
    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    case WM_DESTROY:
        return 0;
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
}