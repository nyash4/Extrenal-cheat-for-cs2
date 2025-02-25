#include "ESP.hpp"
#include "../ImguiMenu/Memory/memory.h"
#include "Offsets.h"
#include <algorithm>

template <typename T>
T Mymax(T a, T b) {
    return (a > b) ? a : b;
}

template <typename T>
T Mymin(T a, T b) {
    return (a < b) ? a : b;
}


// Конструктор
ESP::ESP(EntityManager& entityManager)
    : entityManager(entityManager) {
}

ScreenRes res;

// Деструктор
ESP::~ESP() {}

void ESP::Render() {
    if (!espEnabled) return;

    // Получаем данные игрока
    uintptr_t localPlayer = MemoryUtils::ReadData<uintptr_t>(MemoryUtils::baseAddress + offsets::dwLoclalPlayerPawn);
    if (!localPlayer) return;

    // Получаем матрицу вида
    view_matrix_t viewMatrix = MemoryUtils::ReadData<view_matrix_t>(MemoryUtils::baseAddress + offsets::dwViewMatrix);

    // Получаем размеры окна
    // res.width, res.height;
    glfwGetWindowSize(glfwGetCurrentContext(), &res.width, &res.height);

    // Получаем список сущностей
    auto entities = entityManager.GetEntities();
    int localTeam = entityManager.GetLocalTeam();

    for (const auto& entity : entities) {
        if (!entity.isValid) continue;
        std::cout << localTeam << std::endl;
        if (teamCheck && entity.team == localTeam) continue;

        // Преобразуем мировые координаты в экранные
        Vec3 screenPos = WorldToScreen(entity.position, viewMatrix, res.width, res.height);
        //std::cout << screenPos.x << "\t" << screenPos.y << std::endl;

        // Проверяем, находится ли точка перед камерой
        if (screenPos.z < 0.1f) continue;

        // Определяем цвет в зависимости от команды
        ImColor color = (entity.team != localTeam) ? enemyColor : teamColor;

        // Отрисовываем элементы
        if (drawBoxes) {
            DrawBox(screenPos, color, res.width, res.height);
        }
        if (drawLines) {
            DrawLineToEnemy(screenPos, color, res.width, res.height);
        }
        if(drawFullBodyBoxes) {
            // Вычисляем мировую позицию головы
            Vec3 headScreenPos = WorldToScreen(entity.position, viewMatrix, res.width, res.height);

            // Вычисляем мировую позицию ног.
            // Предполагаем, что ось Z – вертикальная, и голова находится на 50 единиц выше ног.
            Vec3 feetWorldPos = { entity.position.x, entity.position.y, entity.position.z - 50.f };
            Vec3 feetScreenPos = WorldToScreen(feetWorldPos, viewMatrix, res.width, res.height);

            // Если позиция ног некорректна (например, за камерой), пропускаем отрисовку полного бокса
            if (feetScreenPos.z < 0.1f) continue;

            DrawFullBodyBox(headScreenPos, feetScreenPos, color);
        }
    }
}

// Вспомогательные функции
Vec3 ESP::WorldToScreen(const Vec3& worldPos, view_matrix_t matrix, int width, int height) const {
    float _x = matrix[0][0] * worldPos.x + matrix[0][1] * worldPos.y + matrix[0][2] * worldPos.z + matrix[0][3];
    float _y = matrix[1][0] * worldPos.x + matrix[1][1] * worldPos.y + matrix[1][2] * worldPos.z + matrix[1][3];
    float w = matrix[3][0] * worldPos.x + matrix[3][1] * worldPos.y + matrix[3][2] * worldPos.z + matrix[3][3];

    if (w < 0.01f) return { 0, 0, 0 };

    float inv_w = 1.0f / w;
    _x *= inv_w;
    _y *= inv_w;

    float x = width * 0.5f;
    float y = height * 0.5f;

    x += 0.5f * _x * width + 0.5f;
    y -= 0.5f * _y * height + 0.5f;

    return { x, y, w };
}


void ESP::DrawBox(const Vec3& screenPos, const ImColor& color, int width, int height) {
    // Рассчитываем размер бокса в зависимости от расстояния
    float boxSize = static_cast<float>(std::clamp(1000.0 / screenPos.z, 15.0, 50.0));

    ImVec2 Mymin(screenPos.x - boxSize / 2, screenPos.y - boxSize);
    ImVec2 Mymax(screenPos.x + boxSize / 2, screenPos.y);

    // Рисуем прямоугольник
    ImGui::GetBackgroundDrawList()->AddRect(
        Mymin,
        Mymax,
        color,
        0.0f,
        ImDrawFlags_RoundCornersAll,
        boxThickness
    );
}

void ESP::DrawLineToEnemy(const Vec3& screenPos, const ImColor& color, int width, int height) {
    // Линия от нижнего центра экрана к позиции врага
    ImVec2 start(width / 2, height);
    ImVec2 end(screenPos.x, screenPos.y);

    ImGui::GetBackgroundDrawList()->AddLine(
        start,
        end,
        color,
        lineThickness
    );
}

    //box на всё тело
void ESP::DrawFullBodyBox(const Vec3& headScreen, const Vec3& feetScreen, const ImColor& color) {
    // Вычисляем высоту бокса в пикселях
    float boxHeight = feetScreen.y - headScreen.y;
    // Задаём ширину, например, равную половине высоты (можно настроить под себя)
    float boxWidth = boxHeight / 2.0f;

    ImVec2 boxTopLeft(headScreen.x - boxWidth, headScreen.y);
    ImVec2 boxBottomRight(headScreen.x + boxWidth, feetScreen.y);

    ImGui::GetBackgroundDrawList()->AddRect(
        boxTopLeft,
        boxBottomRight,
        color,
        0.0f,
        ImDrawFlags_RoundCornersAll,
        boxThickness
    );
}

// Методы управления состоянием
void ESP::SetEnabled(bool enabled) { espEnabled = enabled; }
bool ESP::IsEnabled() const { return espEnabled; }
void ESP::SetBoxEnabled(bool enabled) { drawBoxes = enabled; }
void ESP::SetFullBoxEnabled(bool enabled) { drawFullBodyBoxes = enabled; }
void ESP::SetLineEnabled(bool enabled) { drawLines = enabled; }
void ESP::SetTeamCheckEnabled(bool enabled) { teamCheck = enabled; }