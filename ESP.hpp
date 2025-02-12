#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../ImguiMenu/Imgui/imgui.h"
#include "EntityData.hpp"
#include "EntityManager.hpp"
#include <vector>
#include <mutex>
#include <atomic>
#include "Resolution.hpp"

struct view_matrix_t{

	float* operator[ ](int index) {
		return matrix[index];
	}

    float matrix[4][4];

};


class ESP {
public:
    ESP(EntityManager& entityManager); // Конструктор, принимающий EntityManager
    ~ESP();

    void Render(); // Основная функция отрисовки ESP
    void SetEnabled(bool enabled); // Включение/выключение ESP
    bool IsEnabled() const; // Проверка, включен ли ESP

    void SetBoxEnabled(bool enabled); // Включение/выключение отрисовки боксов
    void SetLineEnabled(bool enabled); // Включение/выключение отрисовки линий
    void SetTeamCheckEnabled(bool enabled); // Включение/выключение проверки команды

private:
    void DrawBox(const Vec3& screenPos, const ImColor& color, int width, int height); // Отрисовка 3D-бокса
    void DrawLineToEnemy(const Vec3& screenPos, const ImColor& color, int width, int height); // Отрисовка линии до врага
    Vec3 WorldToScreen(const Vec3& worldPos, view_matrix_t matrix, int width, int height) const; // Преобразование мировых координат в экранные

    EntityManager& entityManager; // Ссылка на менеджер сущностей

    std::atomic<bool> espEnabled{ false }; // Включен ли ESP
    std::atomic<bool> drawBoxes{ true }; // Отрисовывать ли боксы
    std::atomic<bool> drawLines{ true }; // Отрисовывать ли линии
    std::atomic<bool> teamCheck{ true }; // Проверять ли команду

    // Параметры отрисовки
    ImColor enemyColor = ImColor(255, 0, 0, 255); // Цвет врагов (красный)
    ImColor teamColor = ImColor(0, 255, 0, 255); // Цвет союзников (зеленый)

    float boxThickness = 1.0f; // Толщина линий бокса
    float lineThickness = 1.0f; // Толщина линий до врагов
};