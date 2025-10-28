#include "library.h"
#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>

// Window procedure for handling basic messages
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

__declspec(dllexport) HWND makeWindow(const char* windowTitle) {
    // Convert UTF-8 or ANSI title to wide string
    wchar_t wideTitle[256];
    int result = MultiByteToWideChar(CP_ACP, 0, windowTitle, -1, wideTitle, 256);
    if (result == 0) {
        MessageBoxW(NULL, L"Failed to convert window title.", L"Error", MB_OK | MB_ICONERROR);
        wcscpy(wideTitle, L"DavinciGL Window"); // fallback title
    }

    // 1. Register the window class
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = TEXT("DavinciGLClass");

    if (!RegisterClassEx(&wc)) {
        MessageBoxW(NULL, L"Failed to register window class.", L"Error", MB_OK | MB_ICONERROR);
        return NULL;
    }

    // 2. Create the window with the dynamic title
    HWND hwnd = CreateWindowEx(
        0,
        TEXT("DavinciGLClass"),
        wideTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    if (!hwnd) {
        MessageBoxW(NULL, L"Failed to create window.", L"Error", MB_OK | MB_ICONERROR);
        return NULL;
    }

    // 3. Show the window
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // 4. Start the rendering loop
    Startrender(hwnd);

    // 5. Message loop
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return hwnd;
}

__declspec(dllexport) void closeWindow(const char* windowName) {
    HWND hwnd = FindWindowA("DavinciGLClass", windowName);
    if (hwnd != NULL) {
        PostMessage(hwnd, WM_CLOSE, 0, 0);
    } else {
        MessageBoxA(NULL, "Window not found.", "Error", MB_OK | MB_ICONERROR);
    }
}