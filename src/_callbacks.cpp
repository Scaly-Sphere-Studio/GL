#include "SSS/GLFW/_callbacks.hpp"
#include "SSS/GLFW/Window.hpp"

__SSS_GLFW_BEGIN
__INTERNAL_BEGIN

// Resizes the internal width and height of correspondig Window instance
void window_resize_callback(GLFWwindow* ptr, int w, int h) try
{
    Window::Shared window = Window::get(ptr);
    window->_w = w;
    window->_h = h;
    glViewport(0, 0, w, h);
}
__CATCH_AND_RETHROW_FUNC_EXC

// Determines current monitor of the window
void window_pos_callback(GLFWwindow* ptr, int x, int y) try
{
    Window::Shared window = Window::get(ptr);

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
}
__CATCH_AND_RETHROW_FUNC_EXC

void monitor_callback(GLFWmonitor* ptr, int event) try
{
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
__SSS_GLFW_END