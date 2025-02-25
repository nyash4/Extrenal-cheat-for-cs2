#include "Radar.hpp"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <regex>
#include <thread>


#define NOMINMAX

Radar::Radar(EntityManager& em) : entityManager(em) {
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}


template <typename T>
T Mymax(T a, T b) {
    return (a > b) ? a : b;
}

Radar::~Radar() {
    if (textureID) glDeleteTextures(1, &textureID);
    Gdiplus::GdiplusShutdown(gdiplusToken);
}

void Radar::Update() {
    // Проверка смены карты
    uintptr_t currentMapPtr = MemoryUtils::ReadData<uintptr_t>(MemoryUtils::baseAddress + offsets::CurrentMap);
    std::string newMap = MemoryUtils::ReadString(currentMapPtr, 11);

    if (newMap != currentMap) {
        currentMap = newMap;
        LoadMapConfig(currentMap);
        LoadMapTexture(currentMap);
    }
}

void Radar::Draw() {
    const ImVec2 SQUARE_MIN_SIZE(200.0f, 200.0f);
    static ImVec2 windowSize = SQUARE_MIN_SIZE;

    // Устанавливаем ограничения с сохранением пропорций 1:1
    ImGui::SetNextWindowSizeConstraints(
        SQUARE_MIN_SIZE,
        ImVec2(FLT_MAX, FLT_MAX),
        [](ImGuiSizeCallbackData* data) {
            // Сохраняем соотношение сторон 1:1
            float targetSize = (std::max)(data->DesiredSize.x, data->DesiredSize.y);
            data->DesiredSize.x = targetSize;
            data->DesiredSize.y = targetSize;
        },
        nullptr
    );

    if (ImGui::Begin("Radar", nullptr,
        ImGuiWindowFlags_NoCollapse)) // Убрать этот флаг, если нужно ручное изменение
    {
        // Получаем текущий размер окна
        windowSize = ImGui::GetWindowSize();

        // Рассчитываем размер текстуры
        const float squareSize = (std::min)(windowSize.x, windowSize.y);

        // Центрируем позицию
        ImVec2 pos = ImGui::GetCursorScreenPos();
        pos.x += (windowSize.x - squareSize) * 0.5f;
        pos.y += (windowSize.y - squareSize) * 0.5f;

        // Рендерим текстуру
        ImGui::GetWindowDrawList()->AddImage(
            (ImTextureID)(intptr_t)textureID,
            pos,
            ImVec2(pos.x + squareSize, pos.y + squareSize),
            ImVec2(0, 0),
            ImVec2(1, 1)
        );

        // Рендерим точки
        auto entities = entityManager.GetEntities();
        int localTeam = entityManager.GetLocalTeam();
        int localPlayerIndex = entityManager.GetLocalPlayerIndex();

        for (int i = 0; i < entities.size(); i++) {
            if (entities[i].isValid) {
                Vec2 screen = WorldToScreen(entities[i].position.x, entities[i].position.y);
                ImVec2 pointPos(
                    pos.x + (screen.x / 1024.0f) * squareSize,
                    pos.y + (screen.y / 1024.0f) * (squareSize) // Инвертируем Y-координату
                );

                ImColor color;
                if (i == localPlayerIndex) {
                    color = ImColor(255, 255, 0); // Желтый для локального игрока
                }
                else {
                    color = (entities[i].team == localTeam)
                        ? ImColor(0, 255, 0)   // Зеленый для союзников
                        : ImColor(255, 0, 0);   // Красный для врагов
                }

                ImGui::GetWindowDrawList()->AddCircleFilled(pointPos, 3.0f, color, 12);
            }
        }
        ImGui::End();
    }
}




// Остальные методы остаются без изменений
void Radar::LoadMapConfig(const std::string& mapName) {
    std::ifstream cfgFile("mapCfg.cfg");
    if (!cfgFile) return;

    std::string line;
    while (std::getline(cfgFile, line)) {
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        if (line.find(mapName) != std::string::npos) {
            std::regex rgx("\"" + mapName + "\"\\{([^}]*)\\}");
            std::smatch match;
            if (std::regex_search(line, match, rgx)) {
                std::string params = match[1].str();
                std::regex numPattern("([a-zA-Z_]+)=([-+]?[0-9]*\\.?[0-9]+)");
                std::smatch numMatch;
                while (std::regex_search(params, numMatch, numPattern)) {
                    std::string key = numMatch[1];
                    double value = std::stod(numMatch[2]);
                    if (key == "pos_xMin") worldXMin = value;
                    else if (key == "pos_xMax") worldXMax = value;
                    else if (key == "pos_yMin") worldYMin = value;
                    else if (key == "pos_yMax") worldYMax = value;
                    params = numMatch.suffix();
                }
            }
        }
    }
}

void Radar::LoadMapTexture(const std::string& mapName) {
    std::wstring path = L"RadarPng/" + std::wstring(mapName.begin(), mapName.end()) + L"_radar.png";

    Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromFile(path.c_str());
    if (bitmap && bitmap->GetLastStatus() == Gdiplus::Ok) {
        imgWidth = 1024;
        imgHeight = 1024;
        aspectRatio = static_cast<float>(imgWidth) / imgHeight;

        // Конвертация в 32bpp ARGB
        Gdiplus::BitmapData bitmapData;
        Gdiplus::Rect rect(0, 0, imgWidth, imgHeight);

        if (bitmap->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bitmapData) == Gdiplus::Ok) {
            // Создание OpenGL текстуры
            if (textureID == 0) {
                glGenTextures(1, &textureID);
            }

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgWidth, imgHeight, 0,
                GL_BGRA, GL_UNSIGNED_BYTE, bitmapData.Scan0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            bitmap->UnlockBits(&bitmapData);
        }

        delete bitmap;
    }
}

Vec2 Radar::WorldToScreen(double X, double Y) const {
    Vec2 screen;
    screen.x = ((X - worldXMin) * imgWidth) / (worldXMax - worldXMin);
    screen.y = ((worldYMax - Y) * imgHeight) / (worldYMax - worldYMin);
    return screen;
}