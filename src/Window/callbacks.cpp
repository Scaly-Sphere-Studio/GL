#include "SSS/GL/Window.hpp"
#include "SSS/Text-Rendering.hpp"
#include "SSS/GL/Objects/Texture.hpp"
#include "SSS/GL/Objects/Camera.hpp"
#include "SSS/GL/Objects/Models/Plane.hpp"

SSS_GL_BEGIN;

// Resizes the internal width and height of correspondig Window instance
void Window::window_resize_callback(GLFWwindow* ptr, int w, int h) try
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
        for (Camera::Weak weak : Camera::_instances) {
            Camera::Shared camera = weak.lock();
            if (camera) {
                camera->_window_ratio = window->getRatio();
                camera->_computeProjection();
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
void Window::window_pos_callback(GLFWwindow* ptr, int x, int y) try
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

void Window::mouse_position_callback(GLFWwindow* ptr, double x, double y)
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

void Window::window_iconify_callback(GLFWwindow* ptr, int state)
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
void Window::mouse_button_callback(GLFWwindow* ptr, int button, int action, int mods) try
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

    // Set focus of Text Area
    if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
        TR::Area::resetFocus();
        Plane::Shared plane = Plane::getHovered();
        if (plane && plane->_use_texture) {
            Texture::Ptr const& texture = window->getObjects().textures.at(plane->getTextureID());
            if (texture->getType() == Texture::Type::Text) {
                TR::Area::Ptr const& area = texture->getTextArea();
                if (area) {
                    int x, y;
                    plane->getRelativeCoords(x, y);
                    area->cursorPlace(x, y);
                }
            }
        }
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
void Window::key_callback(GLFWwindow* ptr, int key, int scancode, int action, int mods) try
{
    Window::Shared const window = Window::get(ptr);

    if (Log::GL::Callbacks::query(Log::GL::Callbacks::get().key)) {
        char buff[256];
        sprintf_s(buff, "'%s' -> Callback -> key (#%d (=%d) -> %d, %d)",
            WINDOW_TITLE(window), key, scancode, action, mods);
        LOG_GL_MSG(buff);
    }
    
    bool const pressed_or_repeat = action == GLFW_PRESS || action == GLFW_REPEAT;
    bool const has_focused_area = TR::Area::getFocused() != nullptr;

    // Fill inputs if no focused text Area
    if (!(has_focused_area && pressed_or_repeat)) {
        // Store key input if in range
        if (key >= 0 && key <= GLFW_KEY_LAST) {
            window->_key_inputs[key] = pressed_or_repeat;
        }
    }

    // Text-Rendering inputs
    if (has_focused_area && pressed_or_repeat) {
        using namespace SSS::TR;
        bool const ctrl = mods & GLFW_MOD_CONTROL;
        switch (key) {
        case GLFW_KEY_ESCAPE:
        case GLFW_KEY_TAB:
            Area::resetFocus();
            break;
        case GLFW_KEY_ENTER:
            Area::cursorAddText("\n");
            break;
        case GLFW_KEY_LEFT:
            Area::cursorMove(ctrl ? Move::CtrlLeft : Move::Left);
            break;
        case GLFW_KEY_RIGHT:
            Area::cursorMove(ctrl ? Move::CtrlRight : Move::Right);
            break;
        case GLFW_KEY_DOWN:
            Area::cursorMove(Move::Down);
            break;
        case GLFW_KEY_UP:
            Area::cursorMove(Move::Up);
            break;
        case GLFW_KEY_HOME:
            Area::cursorMove(Move::Start);
            break;
        case GLFW_KEY_END:
            Area::cursorMove(Move::End);
            break;
        case GLFW_KEY_BACKSPACE:
            Area::cursorDeleteText(ctrl ? Delete::CtrlLeft : Delete::Left);
            break;
        case GLFW_KEY_DELETE:
            Area::cursorDeleteText(ctrl ? Delete::CtrlRight : Delete::Right);
            break;
        }
    }

    // Call user defined callback, if needed
    if (!has_focused_area && window->_key_callback != nullptr) {
        window->_key_callback(ptr, key, scancode, action, mods);
    }
}
CATCH_AND_RETHROW_FUNC_EXC;

// Character input callback
void Window::char_callback(GLFWwindow* ptr, unsigned int codepoint)
{
    Window::Shared const window = Window::get(ptr);

    std::u32string str(1, static_cast<char32_t>(codepoint));
    TR::Area::cursorAddText(str);

    // Call user defined callback, if needed
    if (window->_char_callback != nullptr) {
        window->_char_callback(ptr, codepoint);
    }
}

// Updates connected monitors
void Window::monitor_callback(GLFWmonitor* ptr, int event) try
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

SSS_GL_END;