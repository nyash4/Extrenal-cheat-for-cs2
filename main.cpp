khvjbjbjbk#include "../ImguiMenu/Menu/Menu.hpp"
#include "../ImguiMenu/Radar.hpp"
#include "Resolution.hpp"

int main() {
    glfwInit();;

    ScreenRes res;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GL_TRUE);
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(res.width, res.height+1, "ZOVHook", NULL, NULL);
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

        MainRenderLoop(); // Отрисовка меню

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
