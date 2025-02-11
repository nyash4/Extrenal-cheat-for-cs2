#include "Imgui/imgui.h"
#include "Imgui/imgui_impl_opengl3.h"
#include "Imgui/imgui_impl_glfw.h"

// Важно: include ImGui перед glad и glfw, а glad перед glfw
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
bool show_menu = false;
GLFWwindow* window = nullptr;
HWND hwnd = nullptr;

#ifdef _WIN32
// Функция для настройки кликабельности окна
void set_clickable(bool clickable) {
    LONG style = GetWindowLong(hwnd, GWL_EXSTYLE);
    if (clickable) {
        SetWindowLong(hwnd, GWL_EXSTYLE, style & ~WS_EX_TRANSPARENT);
    }
    else {
        SetWindowLong(hwnd, GWL_EXSTYLE, style | WS_EX_LAYERED | WS_EX_TRANSPARENT);
        SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
    }
}

// Глобальный хук клавиатуры
LRESULT CALLBACK KeyboardHook(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        KBDLLHOOKSTRUCT* kbdStruct = (KBDLLHOOKSTRUCT*)lParam;
        if (wParam == WM_KEYDOWN && kbdStruct->vkCode == VK_INSERT) {
            show_menu = !show_menu;
            set_clickable(show_menu); // Переключаем кликабельность
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// Запуск хука в отдельном потоке
void startKeyboardHook() {
    HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHook, GetModuleHandle(NULL), 0);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnhookWindowsHookEx(hook);
}
#endif

int main() {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GL_TRUE);
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    window = glfwCreateWindow(1920, 1081, "Overlay Window", NULL, NULL);
    if (!window) {
        std::cout << "Fail! Window couldn't get created! \n";
        return 0;
    }

    glfwMakeContextCurrent(window);
    gladLoadGL();
    glViewport(0, 0, 1920, 1080);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    bool active = true;

#ifdef _WIN32
    hwnd = glfwGetWin32Window(window);
    set_clickable(false); // Изначально окно не перехватывает ввод

    // Запускаем перехват клавиш в отдельном потоке
    std::thread keyboardThread(startKeyboardHook);
    keyboardThread.detach();
#endif

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImDrawList* draw_list = ImGui::GetForegroundDrawList();
        ImVec2 p1 = ImVec2(0, 0);
        ImVec2 p2 = ImVec2(1200, 1000);
        draw_list->AddLine(p1, p2, IM_COL32(255, 0, 0, 255), 10.0f);

        if (show_menu) {
            ImGui::Begin("Menu");
            ImGui::Text("Hey, this is nice and easy!");
            ImGui::Checkbox("ESP", &active);

            if (ImGui::Button("!!Close!!")) {
                ImGui_ImplOpenGL3_Shutdown();
                ImGui_ImplGlfw_Shutdown();
                ImGui::DestroyContext();
                glfwDestroyWindow(window);
                glfwTerminate();
                return 0;
            }
            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

#ifdef _WIN32
    UnhookWindowsHookEx(SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHook, GetModuleHandle(NULL), 0));
#endif

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
