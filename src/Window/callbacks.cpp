#include "Window.hpp"
#include "GL/Globals.hpp"

SSS_GL_BEGIN;

_internal::UserCallbacks _internal::user_callbacks;

// Resizes the internal width and height of correspondig Window instance
void Window::window_resize_callback(GLFWwindow* ptr, int w, int h) try
{
    if (Log::GL::Callbacks::query(Log::GL::Callbacks::get().window_resize)) {
        LOG_GL_MSG("Resize (" + toString(w) + "; " + toString(h) + ")");
    }

    // Update internal sizes related values if not iconified (0, 0)
    if (!window->_is_iconified) {
        window->_w = w;
        window->_h = h;
        glViewport(0, 0, w, h);

        // Update screen_ratio of cameras
        for (Camera::Weak weak : Camera::_instances) {
            Camera::Shared camera = weak.lock();
            if (camera) {
                camera->_computeProjection();
            }
        }
    }

    // Call user defined callback, if needed
    if (_internal::user_callbacks.resize != nullptr) {
        _internal::user_callbacks.resize(ptr, w, h);
    }
}
CATCH_AND_RETHROW_FUNC_EXC;

// Determines current monitor of the window
void Window::window_pos_callback(GLFWwindow* ptr, int x, int y) try
{
    if (Log::GL::Callbacks::query(Log::GL::Callbacks::get().window_pos)) {
        char buff[256];
        sprintf_s(buff, "'%s' -> Callback -> position (%d, %d)",
            window->getTitle().c_str(), x, y);
        LOG_GL_MSG(buff);
    }

    window->_x = x;
    window->_y = y;

    if (window->_monitors.size() == 1) {
        window->_setMainMonitor(0);
    }
    else {
        int const x_center = x + window->_w / 2;
        int const y_center = y + window->_h / 2;

        for (int i = 0; i != window->_monitors.size(); ++i) {
            GLFWmonitor* monitor = window->_monitors[i];
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
    if (_internal::user_callbacks.position != nullptr) {
        _internal::user_callbacks.position(ptr, x, y);
    }
}
CATCH_AND_RETHROW_FUNC_EXC;

void Window::mouse_position_callback(GLFWwindow* ptr, double x, double y)
{
    if (Log::GL::Callbacks::query(Log::GL::Callbacks::get().mouse_position)) {
        char buff[256];
        sprintf_s(buff, "'%s' -> Callback -> mouse position (%.1f, %.1f)",
            window->getTitle().c_str(), x, y);
        LOG_GL_MSG(buff);
    }

    // Block inputs
    if (window->_block_inputs)
        return;

    // Call user defined callback, if needed
    if (_internal::user_callbacks.mouse_position != nullptr) {
        _internal::user_callbacks.mouse_position(ptr, x, y);
    }
}

void Window::window_iconify_callback(GLFWwindow* ptr, int state)
{
    window->_is_iconified = static_cast<bool>(state);

    if (Log::GL::Callbacks::query(Log::GL::Callbacks::get().window_iconify)) {
        char buff[256];
        sprintf_s(buff, "'%s' -> Callback -> iconify (%s)",
            window->getTitle().c_str(), toString(window->_is_iconified).c_str());
        LOG_GL_MSG(buff);
    }

    // Call user defined callback, if needed
    if (_internal::user_callbacks.iconify != nullptr) {
        _internal::user_callbacks.iconify(ptr, state);
    }
}

// Used for clickable planes and such
void Window::mouse_button_callback(GLFWwindow* ptr, int button, int action, int mods) try
{
    if (Log::GL::Callbacks::query(Log::GL::Callbacks::get().mouse_button)) {
        char buff[256];
        sprintf_s(buff, "'%s' -> Callback -> mouse button (#%d -> %d, %d)",
            window->getTitle().c_str(), button, action, mods);
        LOG_GL_MSG(buff);
    }

    // Block inputs
    if (window->_block_inputs)
        return;

    // If the cursor is currently moving, update hovering
    if (window->_cursor_is_moving) {
        window->_updateHoveredModel();
    }

    // Set focus of Text Area
    if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
        TR::Area::resetFocus();
        Plane::Shared const plane = window->getHovered<Plane>();
        if (plane && plane->_texture && plane->_texture->getType() == Texture::Type::Text) {
            TR::Area* area = plane->_texture->getTextArea();
            if (area && area->isFocusable()) {
                int x, y;
                plane->getRelativeCoords(x, y);
                area->cursorPlace(x, y);
                // Reset key_inputs
                while (!window->_key_queue.empty()) {
                    window->_key_queue.pop();
                }
                for (auto& key : window->_key_inputs) {
                    key.reset();
                }
            }
        }
    }

    bool const has_focused_area = TR::Area::getFocused();

    // Fill inputs if no focused text Area OR key was released
    if (action != GLFW_REPEAT && (!has_focused_area || action == GLFW_RELEASE)
        && button >= 0 && button < window->_click_inputs.size())
    {
        window->_click_queue.push(std::make_pair(button, action));
    }

    // Call user defined callback, if needed
    if (!has_focused_area && _internal::user_callbacks.mouse_button != nullptr) {
        _internal::user_callbacks.mouse_button(ptr, button, action, mods);
    }
}
CATCH_AND_RETHROW_FUNC_EXC;

// Stores key inputs
void Window::key_callback(GLFWwindow* ptr, int key, int scancode, int action, int mods) try
{
    if (Log::GL::Callbacks::query(Log::GL::Callbacks::get().key)) {
        char buff[256];
        sprintf_s(buff, "'%s' -> Callback -> key (#%d (=%d) -> %d, %d)",
            window->getTitle().c_str(), key, scancode, action, mods);
        LOG_GL_MSG(buff);
    }
    
    if (action == GLFW_PRESS && key == window->_unblocking_key) {
        window->unblockInputs();
    }

    // Block inputs
    if (window->_block_inputs)
        return;

    bool const has_focused_area = TR::Area::getFocused() != nullptr;

    // Fill inputs if no focused text Area OR key was released
    if (action != GLFW_REPEAT && (!has_focused_area || action == GLFW_RELEASE)
        && key >= 0 && key < window->_key_inputs.size())
    {
        window->_key_queue.push(std::make_pair(key, action));
    }

    // Text-Rendering inputs
    if (has_focused_area && action != GLFW_RELEASE) {
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
    if (!has_focused_area && _internal::user_callbacks.key != nullptr) {
        _internal::user_callbacks.key(ptr, key, scancode, action, mods);
    }
}
CATCH_AND_RETHROW_FUNC_EXC;

// Character input callback
void Window::char_callback(GLFWwindow* ptr, unsigned int codepoint)
{
    // Block inputs
    if (window->_block_inputs)
        return;

    std::u32string str(1, static_cast<char32_t>(codepoint));
    TR::Area::cursorAddText(str);

    // Call user defined callback, if needed
    if (_internal::user_callbacks.character != nullptr) {
        _internal::user_callbacks.character(ptr, codepoint);
    }
}

void Window::_retrieveMonitors()
{
    // Clear vector
    _monitors.clear();
    // Retrieve all monitors
    int size;
    GLFWmonitor** arr = glfwGetMonitors(&size);
    // Store pointers in our vector
    _monitors.resize(size);
    for (int i = 0; i < size; i++) {
        _monitors[i] = arr[i];
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

    window->_retrieveMonitors();
}
CATCH_AND_RETHROW_FUNC_EXC;

SSS_GL_END;