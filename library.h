#ifndef LIBRARY_H
#define LIBRARY_H

#include <windows.h>
#include <stdbool.h>
#include "colortypes.h"  // ✅ This must be here

#ifdef __cplusplus
extern "C" {
#endif

    __declspec(dllexport) HWND makeWindow(const char* windowTitle);
    __declspec(dllexport) void closeWindow(const char* windowName);
    __declspec(dllexport) int Startrender(HWND hWnd);
    __declspec(dllexport) void registerSprite(const char* name, const char* path);
    __declspec(dllexport) void drawSpriteByName(const char* name, int x, int y);
    __declspec(dllexport) void addTriangle(Vec3 v0, Vec3 v1, Vec3 v2, Color color);
    __declspec(dllexport) void addRect(Vec3 topLeft, Vec3 bottomRight, Color color);

#ifdef __cplusplus
}
#endif

#endif // LIBRARY_H