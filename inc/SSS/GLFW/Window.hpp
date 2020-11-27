#pragma once

#include "SSS/GLFW/_includes.hpp"
#include "SSS/GLFW/_pointers.hpp"

__SSS_GLFW_BEGIN

class Window {
public:
    // Constructor, creates a window and makes its context current
    Window(int w, int h, std::string const& title = "Untitled");
    // Destructor
    ~Window();

    // Wether the user requested to close the window.
    // NOTE: this simply is a call to glfwWindowShouldClose
    bool shouldClose() const noexcept;

private:
    int _w;
    int _h;
    _internal::GLFWwindow_Ptr _window;
};

__SSS_GLFW_END