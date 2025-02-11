#include "menu.hpp"

int main() {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GL_TRUE);
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(1920, 1081, "Overlay Window", NULL, NULL);
    if (!window) {
        std::cout << "Fail! Window couldn't get created! \n";
        return 0;
    }

    glfwMakeContextCurrent(window);
    gladLoadGL();
    glViewport(0, 0, 1920, 1080);

    InitMenu(window); // Инициализация меню

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Рисуем линию
        ImDrawList* draw_list = ImGui::GetForegroundDrawList();
        ImVec2 p1 = ImVec2(0, 0);
        ImVec2 p2 = ImVec2(1200, 1000);
        draw_list->AddLine(p1, p2, IM_COL32(255, 0, 0, 255), 10.0f);

        RenderMenu(); // Отрисовка меню

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
