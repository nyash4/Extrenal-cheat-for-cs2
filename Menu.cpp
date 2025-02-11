#include "menu.hpp"

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

// Отрисовка меню
void RenderMenu() {
    if (!show_menu) return;

    ImGui::Begin("Menu");
    ImGui::Text("Hey, this is nice and easy!");
    static bool active = true;
    ImGui::Checkbox("ESP", &active);

    if (ImGui::Button("!!Close!!")) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
        exit(0);
    }
    ImGui::End();
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
