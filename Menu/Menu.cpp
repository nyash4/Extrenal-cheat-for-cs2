#include "menu.hpp"
#include "../Radar.hpp"
#include "../EntityManager.hpp"
#include "../ESP.hpp"

// Глобальные переменные
bool show_menu = false;
GLFWwindow* window = nullptr;
HWND hwnd = nullptr;

// Инициализация меню и EntityManager
EntityManager entityManager;       // Создаем менеджер сущностей
Radar radar(entityManager);          // Передаем менеджер в радар
ESP esp(entityManager);              // Создаем объект ESP, используя менеджер сущностей

// Инициализация меню
void InitMenu(GLFWwindow* win) {
    window = win;
    hwnd = glfwGetWin32Window(window);

    // Инициализация ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    SetClickable(false); // Изначально окно не кликабельно

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

    // Устанавливаем ограничения по размеру окна
    ImGui::SetNextWindowSizeConstraints(ImVec2(300, 200), ImVec2(FLT_MAX, FLT_MAX));

    if (ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_NoCollapse)) {
        if (ImGui::BeginTabBar("MainTabs")) {
            // Вкладка AIM
            if (ImGui::BeginTabItem("AIM")) {
                ImGui::Checkbox("Enable Aim Assist", &aimEnabled);
                if (aimEnabled) {
                    ImGui::SliderFloat("Aim Strength", &aimValue, 0.01f, 7.0f, "%.2f");
                }
                ImGui::EndTabItem();
            }

            // Вкладка ESP
            if (ImGui::BeginTabItem("ESP")) {
                // Включение/отключение ESP
                if (ImGui::Checkbox("Enable ESP", &espEnabled)) {
                    esp.SetEnabled(espEnabled);
                }
                if (espEnabled) {
                    // Локальные переменные для настроек ESP
                    static bool boxes = true;
                    static bool lines = true;
                    static bool team_check = true;
                    static bool fullBodyBox = true;
                    if (ImGui::Checkbox("HeadBoxes", &boxes)) {
                        esp.SetBoxEnabled(boxes);
                    }
                    if (ImGui::Checkbox("Lines", &lines)) {
                        esp.SetLineEnabled(lines);
                    }
                    if (ImGui::Checkbox("FullBodyBox", &fullBodyBox)) {
                        esp.SetFullBoxEnabled(fullBodyBox);
                    }
                    if (ImGui::Checkbox("Team Check", &team_check)) {
                        esp.SetTeamCheckEnabled(team_check);
                    }
                    // Цвета для ESP – используем локальные переменные ImVec4.
                    // Если в классе ESP имеются сеттеры для изменения цветов,
                    // здесь можно вызвать их после редактирования.
                    static ImVec4 enemyColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                    static ImVec4 teamColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
                    ImGui::ColorEdit4("Enemy Color", (float*)&enemyColor);
                    ImGui::ColorEdit4("Team Color", (float*)&teamColor);
                }
                ImGui::EndTabItem();
            }

            // Вкладка MISC
            if (ImGui::BeginTabItem("MISC")) {
                ImGui::Checkbox("Radar Hack", &radarEnabled);
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        // Кнопка завершения работы программы
        ImGui::Separator();
        if (ImGui::Button("!!Close Program!!")) {
            // Очистка ресурсов и завершение
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

    // Рендерим радар, если он включен
    if (radarEnabled) {
        radar.Update();  // Обновляем данные радара
        radar.Draw();    // Отрисовываем радар
    }

    // Отрисовываем ESP (внутри него может быть проверка включённости)
    esp.Render();
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
