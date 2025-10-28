#include "library.h"
#include "colortypes.h"
#include <vector>
#include <thread>
#include <unordered_map>
#include <string>
#include <windows.h>
#include <algorithm>
#include <cmath>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// ===== Dynamic Resolution =====
int frameWidth = 0;
int frameHeight = 0;

// ===== Buffers =====
std::vector<Color> backBuffer;

// ===== DIB Resources =====
HBITMAP dib = nullptr;
HDC memDC = nullptr;
uint8_t* dibPixels = nullptr;

// ===== Types =====
struct Sprite {
    int width, height;
    std::vector<Color> pixels;
};

enum ShapeType { SHAPE_TRIANGLE, SHAPE_RECT };

struct Shape {
    ShapeType type;
    std::vector<Vec3> vertices;
    Color color;
};

struct QueuedSprite {
    std::string name;
    int x, y;
};

// ===== Global Queues =====
std::unordered_map<std::string, Sprite> spriteAtlas;
std::vector<Shape> shapeQueue;
std::vector<QueuedSprite> spriteQueue;

// ===== Resolution Update =====
void updateResolution(HWND hWnd) {
    RECT rect;
    GetClientRect(hWnd, &rect);
    int newWidth = rect.right - rect.left;
    int newHeight = rect.bottom - rect.top;

    if (newWidth == frameWidth && newHeight == frameHeight) return;

    frameWidth = newWidth;
    frameHeight = newHeight;
    backBuffer.resize(frameWidth * frameHeight);

    if (dib) DeleteObject(dib);
    if (memDC) DeleteDC(memDC);

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = frameWidth;
    bmi.bmiHeader.biHeight = -frameHeight;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biCompression = BI_RGB;

    HDC hdc = GetDC(hWnd);
    dib = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (void**)&dibPixels, NULL, 0);
    memDC = CreateCompatibleDC(hdc);
    SelectObject(memDC, dib);
    ReleaseDC(hWnd, hdc);
}

// ===== API Functions =====
extern "C" __declspec(dllexport)
void registerSprite(const char* name, const char* path) {
    int w, h, channels;
    stbi_uc* data = stbi_load(path, &w, &h, &channels, 3);
    if (!data || w <= 0 || h <= 0) return;

    Sprite sprite;
    sprite.width = w;
    sprite.height = h;
    sprite.pixels.resize(w * h);

    for (int i = 0; i < w * h; ++i) {
        sprite.pixels[i] = {
            data[i * channels + 0],
            data[i * channels + 1],
            data[i * channels + 2]
        };
    }

    stbi_image_free(data);
    spriteAtlas[name] = sprite;
}

extern "C" __declspec(dllexport)
void drawSpriteByName(const char* name, int x, int y) {
    spriteQueue.push_back({name, x, y});
}

extern "C" __declspec(dllexport)
void addTriangle(Vec3 v0, Vec3 v1, Vec3 v2, Color color) {
    shapeQueue.push_back({SHAPE_TRIANGLE, {v0, v1, v2}, color});
}

extern "C" __declspec(dllexport)
void addRect(Vec3 topLeft, Vec3 bottomRight, Color color) {
    Vec3 v0 = topLeft;
    Vec3 v1 = {bottomRight.x, topLeft.y, 0};
    Vec3 v2 = bottomRight;
    Vec3 v3 = {topLeft.x, bottomRight.y, 0};
    shapeQueue.push_back({SHAPE_RECT, {v0, v1, v2, v3}, color});
}

// ===== Internal Drawing =====
POINT project(const Vec3& v) {
    return {
        static_cast<int>(v.x + frameWidth / 2),
        static_cast<int>(v.y + frameHeight / 2)
    };
}

void drawTriangle(const Vec3& v0, const Vec3& v1, const Vec3& v2, Color color) {
    POINT p0 = project(v0);
    POINT p1 = project(v1);
    POINT p2 = project(v2);

    int minX = std::max(0, std::min({(int)p0.x, (int)p1.x, (int)p2.x}));
    int maxX = std::min(frameWidth - 1, std::max({(int)p0.x, (int)p1.x, (int)p2.x}));
    int minY = std::max(0, std::min({(int)p0.y, (int)p1.y, (int)p2.y}));
    int maxY = std::min(frameHeight - 1, std::max({(int)p0.y, (int)p1.y, (int)p2.y}));

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            backBuffer[y * frameWidth + x] = color;
        }
    }
}

void drawSprite(const Sprite& sprite, int posX, int posY) {
    for (int y = 0; y < sprite.height; ++y) {
        for (int x = 0; x < sprite.width; ++x) {
            int dstX = posX + x;
            int dstY = posY + y;
            if (dstX >= 0 && dstX < frameWidth && dstY >= 0 && dstY < frameHeight) {
                backBuffer[dstY * frameWidth + dstX] = sprite.pixels[y * sprite.width + x];
            }
        }
    }
}

// ===== Frame Rendering =====
void renderFrame(HWND hWnd, HDC hdc) {
    updateResolution(hWnd);
    std::fill(backBuffer.begin(), backBuffer.end(), Color{192, 192, 192});

    for (const Shape& shape : shapeQueue) {
        if (shape.type == SHAPE_TRIANGLE && shape.vertices.size() == 3) {
            drawTriangle(shape.vertices[0], shape.vertices[1], shape.vertices[2], shape.color);
        } else if (shape.type == SHAPE_RECT && shape.vertices.size() == 4) {
            drawTriangle(shape.vertices[0], shape.vertices[1], shape.vertices[2], shape.color);
            drawTriangle(shape.vertices[2], shape.vertices[3], shape.vertices[0], shape.color);
        }
    }

    for (const QueuedSprite& s : spriteQueue) {
        auto it = spriteAtlas.find(s.name);
        if (it != spriteAtlas.end()) {
            drawSprite(it->second, s.x, s.y);
        }
    }

    for (int y = 0; y < frameHeight; ++y) {
        for (int x = 0; x < frameWidth; ++x) {
            Color c = backBuffer[y * frameWidth + x];
            int index = (y * frameWidth + x) * 3;
            dibPixels[index + 0] = c.b;
            dibPixels[index + 1] = c.g;
            dibPixels[index + 2] = c.r;
        }
    }

    BitBlt(hdc, 0, 0, frameWidth, frameHeight, memDC, 0, 0, SRCCOPY);
    spriteQueue.clear();
}

// ===== Entry Point =====
extern "C" __declspec(dllexport)
int Startrender(HWND hWnd) {
    HDC hdc = GetDC(hWnd);
    if (!hdc) return 1;

    std::thread render_thread([=]() {
        while (IsWindow(hWnd)) {
            renderFrame(hWnd, hdc);
            Sleep(16); // ~60 FPS
        }
        ReleaseDC(hWnd, hdc);
        DeleteDC(memDC);
        DeleteObject(dib);
    });

    render_thread.detach();
    return 0;
}