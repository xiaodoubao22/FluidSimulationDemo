#include "Window.h"

namespace Fw {
    Window::Window(int width, int height, const std::string& title) : mWindow(nullptr) {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        mWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if (!mWindow) {
            glfwTerminate();
            return;
        }
        glfwMakeContextCurrent(mWindow);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            glfwTerminate();
            return;
        }
    }

    Window::~Window() {
        if (mWindow) {
            glfwDestroyWindow(mWindow);
        }
        glfwTerminate();
    }

    bool Window::ShouldClose() const {
        return glfwWindowShouldClose(mWindow);
    }

    void Window::SwapBuffers() const {
        glfwSwapBuffers(mWindow);
    }

    void Window::PollEvents() const {
        glfwPollEvents();
    }

    void Window::SetShouldClose(bool value) {
        glfwSetWindowShouldClose(mWindow, value);
    }

    void Window::SetTitle(const std::string& title) const {
        glfwSetWindowTitle(mWindow, title.c_str());
    }
}
