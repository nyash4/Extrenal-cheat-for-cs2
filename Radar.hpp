#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../ImguiMenu/Imgui/imgui.h"
#include <string>
#include <vector>
#include <atomic>
#include <mutex>
#include "offsets.h"
#include "memory.h"
#include <Windows.h>
#include <gdiplus.h>


#pragma comment(lib, "gdiplus.lib")


struct Vec2 {
    double x, y;
};

struct EntityData {
    Vec2 position;
    bool isValid;
    int team;
};

class Radar {
private:
    ULONG_PTR gdiplusToken;
    std::vector<EntityData> entities;
    std::atomic<int> g_LocalTeam{ 0 };
    std::atomic<int> g_localPlayerIndex{ 0 };
    std::mutex entitiesMutex;

    GLuint textureID = 0;
    int imgWidth = 0, imgHeight = 0;
    std::string currentMap;

    double worldXMin = -3230, worldXMax = 3230;
    double worldYMin = -3230, worldYMax = 3230;


    std::atomic<bool> running{ true };
    std::thread updateThread;

public:
    Radar();
    ~Radar();

    void Update();
    void Draw();
    void LoadMapConfig(const std::string& mapName);
    void LoadMapTexture(const std::string& mapName);

    ImVec2 minRadarSize = ImVec2(100.0f, 100.0f); // Минимальный размер
    float aspectRatio = 1.0f; // Соотношение сторон

private:
    void EntityUpdateThread();
    Vec2 WorldToScreen(double X, double Y) const;
};