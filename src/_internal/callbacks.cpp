#include "SSS/GL/_internal/callbacks.hpp"
#include "SSS/GL/Window.hpp"

__SSS_GL_BEGIN

namespace LOG {
    bool internal_callbacks::window_resize{ false };
    bool internal_callbacks::window_pos{ false };
    bool internal_callbacks::mouse_position{ false };
    bool internal_callbacks::mouse_button{ false };
    bool internal_callbacks::key{ false };
    bool internal_callbacks::monitor{ false };
}

__INTERNAL_BEGIN

// Resizes the internal width and height of correspondig Window instance
void window_resize_callback(GLFWwindow* ptr, int w, int h) try
{
    if (LOG::internal_callbacks::window_resize) {
        __LOG_FUNC_MSG(toString(w) + 'x' + toString(h));
    }

    Window::Shared window = Window::get(ptr);
    window->use();
    window->_w = w;
    window->_h = h;
    window->_setProjections();
    glViewport(0, 0, w, h);

    // Update scaling of Buttons adapting to screen ratio
    for (auto const& weak : Button::_instances) {
        Button::Shared button = weak.lock();
        if (button) {
            button->_updateWinScaling();
        }
    }

    // Call user defined callback, if needed
    if (window->_resize_callback != nullptr) {
        window->_resize_callback(ptr, w, h);
    }
}
__CATCH_AND_RETHROW_FUNC_EXC

// Determines current monitor of the window
void window_pos_callback(GLFWwindow* ptr, int x, int y) try
{
    if (LOG::internal_callbacks::window_pos) {
        __LOG_FUNC_MSG(toString(x) + 'x' + toString(y));
    }

    Window::Shared window = Window::get(ptr);
    window->use();

    if (Window::_monitors.size() == 1) {
        if (Window::_monitors[0].ptr != window->_main_monitor.ptr) {
            window->_setMainMonitor(Window::_monitors[0]);
        }
        return;
    }

    int const x_center = x + window->_w / 2;
    int const y_center = y + window->_h / 2;

    for (Monitor const& monitor : Window::_monitors) {
        int x0, y0, xmax, ymax;
        glfwGetMonitorPos(monitor.ptr, &x0, &y0);
        GLFWvidmode const* mode = glfwGetVideoMode(monitor.ptr);
        xmax = x0 + mode->width;
        ymax = y0 + mode->height;
        if (x0 <= x_center && xmax > x_center && y0 <= y_center && ymax > y_center) {
            if (monitor.ptr != window->_main_monitor.ptr) {
                window->_setMainMonitor(monitor);
            }
            break;
        }
    }
    // Call user defined callback, if needed
    if (window->_pos_callback != nullptr) {
        window->_pos_callback(ptr, x, y);
    }
}
__CATCH_AND_RETHROW_FUNC_EXC

// Used for clickable buttons and such
void mouse_position_callback(GLFWwindow* ptr, double x, double y)
{
    if (LOG::internal_callbacks::mouse_position) {
        __LOG_FUNC_MSG(toString(x) + 'x' + toString(y));
    }

    Window::Shared window = Window::get(ptr);
    window->use();

    // Get mouse coordinates in -1; 1 range
    x = (x / static_cast<double>(window->_w) * 2.0) - 1.0;
    y = ((y / static_cast<double>(window->_h) * 2.0) - 1.0) * -1.0;

    // Update button hover status
    for (auto const& weak : Button::_instances) {
        Button::Shared button = weak.lock();
        if (button) {
            button->_updateHoverStatus(x, y);
        }
    }

    // Call user defined callback, if needed
    if (window->_mouse_position_callback != nullptr) {
        window->_mouse_position_callback(ptr, x, y);
    }
}

// Used for clickable buttons and such
void mouse_button_callback(GLFWwindow* ptr, int button, int action, int mods) try
{
    if (LOG::internal_callbacks::mouse_button) {
        __LOG_FUNC_MSG(context_msg("button", toString(button))
            + "; " + context_msg("action", toString(action))
            + "; " + context_msg("mods", toString(mods))
        );
    }

    Window::Shared window = Window::get(ptr);
    window->use();

    // Call button functions, if needed
    if (action == GLFW_PRESS) {
        for (auto const& weak : Button::_instances) {
            Button::Shared button = weak.lock();
            if (button && button->isHovered()) {
                button->callFunction();
            }
        }
    }

    // Call user defined callback, if needed
    if (window->_mouse_button_callback != nullptr) {
        window->_mouse_button_callback(ptr, button, action, mods);
    }
}
__CATCH_AND_RETHROW_FUNC_EXC

// Stores key inputs
void key_callback(GLFWwindow* ptr, int key, int scancode, int action, int mods) try
{
    if (LOG::internal_callbacks::key) {
        __LOG_FUNC_MSG(context_msg("key", toString(key))
            + "; " + context_msg("scancode", toString(scancode))
            + "; " + context_msg("action", toString(action))
            + "; " + context_msg("mods", toString(mods))
        );
    }

    Window::Shared window = Window::get(ptr);
    window->use();

    window->_key_inputs[key] = action != GLFW_RELEASE;

    // Call user defined callback, if needed
    if (window->_key_callback != nullptr) {
        window->_key_callback(ptr, key, scancode, action, mods);
    }
}
__CATCH_AND_RETHROW_FUNC_EXC

// Updates connected monitors
void monitor_callback(GLFWmonitor* ptr, int event) try
{
    if (LOG::internal_callbacks::monitor) {
        __LOG_FUNC_MSG(context_msg("event", toString(event)));
    }
    // Ignore arguments
    ptr; event;
    // Clear vector
    Window::_monitors.clear();

    // Retrieve all monitors
    int size;
    GLFWmonitor** arr = glfwGetMonitors(&size);
    // Store pointers & additional info in our vector
    Window::_monitors.resize(size);
    for (int i = 0; i < size; i++) {
        Monitor& monitor = Window::_monitors[i];
        // Store pointer
        monitor.ptr = arr[i];
        // Retrieve screen dimensions (in millimeters)
        int width_mm, height_mm;
        glfwGetMonitorPhysicalSize(monitor.ptr, &width_mm, &height_mm);
        // Store dimensions (in inches)
        monitor.w = static_cast<float>(width_mm) * 0.0393701f;
        monitor.h = static_cast<float>(height_mm) * 0.0393701f;
    }
}
__CATCH_AND_RETHROW_FUNC_EXC

__INTERNAL_END
__SSS_GL_END