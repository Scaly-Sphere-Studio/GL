#include "SSS/GLFW/Window.hpp"

__SSS_GLFW_BEGIN

// Constructor, creates a window and makes its context current
Window::Window(int w, int h, std::string const& title) try
    : _w(w), _h(h)
{
    // Create window
    _window.reset(glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr));
    // Throw if an error occured
    if (!_window) {
        const char* msg;
        glfwGetError(&msg);
        throw_exc(msg);
    }

    // Initialize GLEW
    // TODO : understand wtf this does
    glfwMakeContextCurrent(_window.get());

    __LOG_CONSTRUCTOR
}
__CATCH_AND_RETHROW_METHOD_EXC

// Destructor
Window::~Window()
{
    __LOG_DESTRUCTOR
}

// Wether the user requested to close the window.
// NOTE: this simply is a call to glfwWindowShouldClose
bool Window::shouldClose() const noexcept
{
    return glfwWindowShouldClose(_window.get());
}

__SSS_GLFW_END