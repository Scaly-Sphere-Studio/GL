#include "SSS/GL/_internal/callbacks.hpp"
#include "SSS/GL/Window.hpp"

SSS_GL_BEGIN;
INTERNAL_BEGIN;

// Resizes the internal width and height of correspondig Window instance
void window_resize_callback(GLFWwindow* ptr, int w, int h) try
{
    Window::Shared const window = Window::get(ptr);

    if (Log::GL::Callbacks::query(Log::GL::Callbacks::get().window_resize)) {
        char buff[256];
        sprintf_s(buff, "'%s' -> Callback -> resize (%d, %d)",
            WINDOW_TITLE(window), w, h);
        LOG_GL_MSG(buff);
    }

    // Update internal sizes related values if not iconified (0, 0)
    if (!window->_is_iconified) {
        window->_w = w;
        window->_h = h;
        {
            Context const context(window);
            glViewport(0, 0, w, h);
        }

        // Update screen_ratio of cameras
        auto const& cameras = window->_objects.cameras;
        for (auto it = cameras.cbegin(); it != cameras.cend(); ++it) {
            if (it->second) {
                it->second->_screen_ratio = window->getScreenRatio();
                it->second->_computeProjection();
            }
        }
    }

    // Call user defined callback, if needed
    if (window->_resize_callback != nullptr) {
        window->_resize_callback(ptr, w, h);
    }
}
CATCH_AND_RETHROW_FUNC_EXC;

// Determines current monitor of the window
void window_pos_callback(GLFWwindow* ptr, int x, int y) try
{
    Window::Shared const window = Window::get(ptr);

    if (Log::GL::Callbacks::query(Log::GL::Callbacks::get().window_pos)) {
        char buff[256];
        sprintf_s(buff, "'%s' -> Callback -> position (%d, %d)",
            WINDOW_TITLE(window), x, y);
        LOG_GL_MSG(buff);
    }

    if (Window::_monitors.size() == 1) {
        window->_setMainMonitor(0);
    }
    else {
        int const x_center = x + window->_w / 2;
        int const y_center = y + window->_h / 2;

        for (int i = 0; i != Window::_monitors.size(); ++i) {
            GLFWmonitor* monitor = Window::_monitors[i];
            int x0, y0, xmax, ymax;
            glfwGetMonitorPos(monitor, &x0, &y0);
            GLFWvidmode const* mode = glfwGetVideoMode(monitor);
            xmax = x0 + mode->width;
            ymax = y0 + mode->height;
            if (x0 <= x_center && xmax > x_center && y0 <= y_center && ymax > y_center) {
                if (monitor != window->_main_monitor) {
                    window->_setMainMonitor(i);
                }
                break;
            }
        }
    }

    // Call user defined callback, if needed
    if (window->_pos_callback != nullptr) {
        window->_pos_callback(ptr, x, y);
    }
}
CATCH_AND_RETHROW_FUNC_EXC;

void mouse_position_callback(GLFWwindow* ptr, double x, double y)
{
    Window::Shared const window = Window::get(ptr);

    if (Log::GL::Callbacks::query(Log::GL::Callbacks::get().mouse_position)) {
        char buff[256];
        sprintf_s(buff, "'%s' -> Callback -> mouse position (%.1f, %.1f)",
            WINDOW_TITLE(window), x, y);
        LOG_GL_MSG(buff);
    }

    // Call user defined callback, if needed
    if (window->_mouse_position_callback != nullptr) {
        window->_mouse_position_callback(ptr, x, y);
    }
}

void window_iconify_callback(GLFWwindow* ptr, int state)
{
    Window::Shared const window = Window::get(ptr);
    window->_is_iconified = static_cast<bool>(state);

    if (Log::GL::Callbacks::query(Log::GL::Callbacks::get().window_iconify)) {
        char buff[256];
        sprintf_s(buff, "'%s' -> Callback -> iconify (%s)",
            WINDOW_TITLE(window), toString(window->_is_iconified).c_str());
        LOG_GL_MSG(buff);
    }

    // Call user defined callback, if needed
    if (window->_iconify_callback != nullptr) {
        window->_iconify_callback(ptr, state);
    }
}

// Used for clickable planes and such
void mouse_button_callback(GLFWwindow* ptr, int button, int action, int mods) try
{
    Window::Shared const window = Window::get(ptr);

    if (Log::GL::Callbacks::query(Log::GL::Callbacks::get().mouse_button)) {
        char buff[256];
        sprintf_s(buff, "'%s' -> Callback -> mouse button (#%d -> %d, %d)",
            WINDOW_TITLE(window), button, action, mods);
        LOG_GL_MSG(buff);
    }

    // If the cursor is currently moving, update hovering
    if (window->_cursor_is_moving) {
        window->_updateHoveredModel();
    }
    // Call button function, if needed
    window->_callOnClickFunction(button, action, mods);

    // Call user defined callback, if needed
    if (window->_mouse_button_callback != nullptr) {
        window->_mouse_button_callback(ptr, button, action, mods);
    }
}
CATCH_AND_RETHROW_FUNC_EXC;

// Stores key inputs
void key_callback(GLFWwindow* ptr, int key, int scancode, int action, int mods) try
{
    Window::Shared const window = Window::get(ptr);

    if (Log::GL::Callbacks::query(Log::GL::Callbacks::get().key)) {
        char buff[256];
        sprintf_s(buff, "'%s' -> Callback -> key (#%d (=%d) -> %d, %d)",
            WINDOW_TITLE(window), key, scancode, action, mods);
        LOG_GL_MSG(buff);
    }
    
    // Store key input if in range
    if (key >= 0 && key <= GLFW_KEY_LAST) {
        window->_key_inputs[key] = action != GLFW_RELEASE;
    }

    // Call user defined callback, if needed
    if (window->_key_callback != nullptr) {
        window->_key_callback(ptr, key, scancode, action, mods);
    }
}
CATCH_AND_RETHROW_FUNC_EXC;

// Updates connected monitors
void monitor_callback(GLFWmonitor* ptr, int event) try
{
    if (Log::GL::Callbacks::query(Log::GL::Callbacks::get().monitor)) {
        char buff[256];
        sprintf_s(buff, "monitor callback (%d)", event);
        LOG_GL_MSG(buff);
    }
    // Ignore arguments
    ptr; event;

    // Clear vector
    Window::_monitors.clear();
    // Retrieve all monitors
    int size;
    GLFWmonitor** arr = glfwGetMonitors(&size);
    // Store pointers in our vector
    Window::_monitors.resize(size);
    for (int i = 0; i < size; i++) {
        Window::_monitors[i] = arr[i];
    }
}
CATCH_AND_RETHROW_FUNC_EXC;

INTERNAL_END;
SSS_GL_END;