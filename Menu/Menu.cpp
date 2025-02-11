#include "menu.hpp"
#include "../Radar.hpp"

Radar radar;

bool show_menu = false;
GLFWwindow* window = nullptr;
HWND hwnd = nullptr;

// Инициализация меню
void InitMenu(GLFWwindow* win) {
    window = win;
    hwnd = glfwGetWin32Window(window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    SetClickable(false); // Изначально окно прозрачное

#ifdef _WIN32
    std::thread keyboardThread(StartKeyboardHook);
    keyboardThread.detach();
#endif
}

static bool radarEnabled = false;

// Отрисовка меню
void RenderMenu() {
    if (!show_menu) return;

    static bool espEnabled = false;
    static bool aimEnabled = false;
    static float aimValue = 1.0f;
   

    // Set constraints before window creation
    ImGui::SetNextWindowSizeConstraints(ImVec2(300, 200), ImVec2(FLT_MAX, FLT_MAX));

    if (ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_NoCollapse)) {
        if (ImGui::BeginTabBar("MainTabs")) {
            // AIM Tab
            if (ImGui::BeginTabItem("AIM")) {
                ImGui::Checkbox("Enable Aim Assist", &aimEnabled);
                if (aimEnabled) {
                    ImGui::SliderFloat("Aim Strength", &aimValue, 0.01f, 7.0f, "%.2f");
                }
                ImGui::EndTabItem();
            }

            // ESP Tab
            if (ImGui::BeginTabItem("ESP")) {
                ImGui::Checkbox("Enable ESP", &espEnabled);
                ImGui::EndTabItem();
            }

            // MISC Tab
            if (ImGui::BeginTabItem("MISC")) {
                ImGui::Checkbox("Radar Hack", &radarEnabled);
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        // Close button
        ImGui::Separator();
        if (ImGui::Button("!!Close Program!!")) {
            // Cleanup code
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
            glfwDestroyWindow(window);
            glfwTerminate();
            exit(0);
        }
        ImGui::End();
    }
}

void MainRenderLoop() {
    // Рендерим меню
    RenderMenu();

    // Рендерим радар независимо от меню
    if (radarEnabled) {
        radar.Update();
        radar.Draw();
    }
}

// Установка кликабельности окна
void SetClickable(bool clickable) {
    LONG style = GetWindowLong(hwnd, GWL_EXSTYLE);
    if (clickable) {
        SetWindowLong(hwnd, GWL_EXSTYLE, style & ~WS_EX_TRANSPARENT);
    }
    else {
        SetWindowLong(hwnd, GWL_EXSTYLE, style | WS_EX_LAYERED | WS_EX_TRANSPARENT);
        SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
    }
}

#ifdef _WIN32
// Глобальный хук клавиатуры
LRESULT CALLBACK KeyboardHook(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        KBDLLHOOKSTRUCT* kbdStruct = (KBDLLHOOKSTRUCT*)lParam;
        if (wParam == WM_KEYDOWN && kbdStruct->vkCode == VK_INSERT) {
            show_menu = !show_menu;
            SetClickable(show_menu);
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// Запуск хука в отдельном потоке
void StartKeyboardHook() {
    HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHook, GetModuleHandle(NULL), 0);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnhookWindowsHookEx(hook);
}
#endif
