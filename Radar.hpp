#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../ImguiMenu/Imgui/imgui.h"
#include <string>
#include <atomic>
#include <mutex>
#include "EntityData.hpp"
#include "EntityManager.hpp"
#include "offsets.h"
#include "memory.h"
#include <Windows.h>
#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")

struct Vec2 {
    double x, y;
};

class Radar {
private:
    ULONG_PTR gdiplusToken;
    EntityManager& entityManager;
    GLuint textureID = 0;
    int imgWidth = 0, imgHeight = 0;
    std::string currentMap;
    double worldXMin = -3230, worldXMax = 3230;
    double worldYMin = -3230, worldYMax = 3230;

   

public:
    Radar(EntityManager& em);
    ~Radar();

    void Update();
    void Draw();
    void LoadMapConfig(const std::string& mapName);
    void LoadMapTexture(const std::string& mapName);

    ImVec2 minRadarSize = ImVec2(100.0f, 100.0f); // Минимальный размер
    float aspectRatio = 1.0f;

private:
    Vec2 WorldToScreen(double X, double Y) const;
};