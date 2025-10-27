#include <windows.h>
#include <wingdi.h>
#include "library.h"
#include <tlhelp32.h>

// DavinciGL is a Graphics Library built for Simplicity and Peformance.
// It is primarily targetted to Windows and the DirectX API.
// It is built to have minimal overhead like Vulkan yet being simpler than OpenGL.
__declspec(dllexport) void makeWindow(void) {
    HDC screen = GetDC(NULL);  // Get the device context for the entire screen

    if (screen == NULL) {
        MessageBox(NULL, "Failed to get screen device context.", "Error", MB_OK | MB_ICONERROR);
        return;
    }


    while (1) {
        Rectangle(screen, 50, 50, 500, 500);
        Sleep(20);
    }
}

__declspec(dllexport) void closeWindow(const char* windowName) {
    HWND hwnd = FindWindowA(NULL, windowName);
    if (hwnd == NULL) {
        MessageBoxA(NULL, "Window not found.", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    // Get the process ID
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);

    if (pid == 0) {
        MessageBoxA(NULL, "Failed to get process ID.", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    // terminate the process
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess == NULL) {
        MessageBoxA(NULL, "Failed to open process.", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    TerminateProcess(hProcess, 0);
    CloseHandle(hProcess);
}
