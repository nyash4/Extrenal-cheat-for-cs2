#ifndef MENU_HPP
#define MENU_HPP

#include "../Imgui/imgui.h"
#include "../Imgui/imgui_impl_opengl3.h"
#include "../Imgui/imgui_impl_glfw.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <windows.h>
#include <GLFW/glfw3native.h>
#endif

#include <iostream>
#include <thread>

// Глобальные переменные
extern bool show_menu;
extern GLFWwindow* window;

// Инициализация меню
void InitMenu(GLFWwindow* win);

// Отрисовка меню
void RenderMenu();

// Функция для управления кликабельностью окна
void SetClickable(bool clickable);

void MainRenderLoop();

#ifdef _WIN32
// Функция для запуска глобального хука клавиатуры
void StartKeyboardHook();
#endif

#endif // MENU_HPP
