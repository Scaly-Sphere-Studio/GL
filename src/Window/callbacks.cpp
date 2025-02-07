#include "GL/Window.hpp"
#include "GL/Objects/Camera.hpp"
#include "GL/Objects/Models/Plane.hpp"

SSS_GL_BEGIN;

// Resizes the internal width and height of correspondig Window instance
void Window::window_resize_callback(GLFWwindow* ptr, int w, int h) try
{
    Window* window = get(ptr);
    if (!window)
        return;
    Context const context = window->setContext();

    if (Log::GL::Callbacks::query(Log::GL::Callbacks::get().window_resize)) {
        LOG_GL_MSG("Resize (" + toString(w) + "; " + toString(h) + ")");
    }


    // Update internal sizes related values if not iconified (0, 0)
    if (!window->_is_iconified) {
        window->_w = w;
        window->_h = h;
        glViewport(0, 0, w, h);

        // Update screen_ratio of cameras
        for (Camera::Shared camera : Camera::getInstances()) {
            if (camera) {
                camera->_computeProjections();
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
    Window* window = get(ptr);
    if (!window)
        return;
    Context const context = window->setContext();

    if (Log::GL::Callbacks::query(Log::GL::Callbacks::get().window_pos)) {
        char buff[256];
        sprintf_s(buff, "'%s' -> Callback -> position (%d, %d)",
            window->getTitle().c_str(), x, y);
        LOG_GL_MSG(buff);
    }

    window->_x = x;
    window->_y = y;

    if (_main._monitors.size() == 1) {
        window->_setMainMonitor(0);
    }
    else {
        int const x_center = x + window->_w / 2;
        int const y_center = y + window->_h / 2;

        for (int i = 0; i != _main._monitors.size(); ++i) {
            GLFWmonitor* monitor = _main._monitors[i];
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
    Window* window = get(ptr);
    if (!window)
        return;
    Context const context = window->setContext();

    if (Log::GL::Callbacks::query(Log::GL::Callbacks::get().mouse_position)) {
        char buff[256];
        sprintf_s(buff, "'%s' -> Callback -> mouse position (%.1f, %.1f)",
            window->getTitle().c_str(), x, y);
        LOG_GL_MSG(buff);
    }

    // TR
    auto area = TR::Area::getFocused();
    auto plane = window->getHeld<Plane>();
    if (area && plane && plane->_texture && plane->_texture->getType() == Texture::Type::Text
        && plane->_texture->getTextArea() == area)
    {
        int x2, y2;
        plane->getRelativeCoords(x2, y2);
        area->cursorPlace(x2, y2);
    }

    // Block inputs
    if (window->_block_inputs)
        return;

    // Call user defined callback, if needed
    if (window->_mouse_position_callback != nullptr) {
        window->_mouse_position_callback(ptr, x, y);
    }
}

void Window::window_iconify_callback(GLFWwindow* ptr, int state)
{
    Window* window = get(ptr);
    if (!window)
        return;
    Context const context = window->setContext();

    if (Log::GL::Callbacks::query(Log::GL::Callbacks::get().window_iconify)) {
        char buff[256];
        sprintf_s(buff, "'%s' -> Callback -> iconify (%s)",
            window->getTitle().c_str(), toString(window->_is_iconified).c_str());
        LOG_GL_MSG(buff);
    }

    window->_is_iconified = static_cast<bool>(state);

    // Call user defined callback, if needed
    if (window->_iconify_callback != nullptr) {
        window->_iconify_callback(ptr, state);
    }
}

// Used for clickable planes and such
void Window::mouse_button_callback(GLFWwindow* ptr, int button, int action, int mods) try
{
    Window* window = get(ptr);
    if (!window)
        return;
    Context const context = window->setContext();

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
            else
                TR::Area::resetFocus();
        }
        else
            TR::Area::resetFocus();
    }
    else if (auto area = TR::Area::getFocused(); area && button == GLFW_MOUSE_BUTTON_1
        && action == GLFW_RELEASE && !window->keyMod(GLFW_MOD_SHIFT))
    {
        area->unlockSelection();
    }

    // Fill inputs if no focused text Area OR key was released
    if (action != GLFW_REPEAT && button >= 0 && button < window->_click_inputs.size()) {
        window->_click_queue.push(std::make_pair(button, action));
    }

    // Call user defined callback, if needed
    if (window->_mouse_button_callback != nullptr) {
        window->_mouse_button_callback(ptr, button, action, mods);
    }
}
CATCH_AND_RETHROW_FUNC_EXC;

// Stores key inputs
void Window::key_callback(GLFWwindow* ptr, int key, int scancode, int action, int mods) try
{
    Window* window = get(ptr);
    if (!window)
        return;
    Context const context = window->setContext();

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

    if (auto area = TR::Area::getFocused(); area
        && (window->_key_mods & GLFW_MOD_SHIFT) != (mods & GLFW_MOD_SHIFT))
    {
        if ((mods & GLFW_MOD_SHIFT) != 0)
            area->lockSelection();
        else if (auto plane = window->getHeld<Plane>(); !plane || !plane->_texture
            || plane->_texture->_type != Texture::Type::Text || plane->_texture->getTextArea() != area)
            area->unlockSelection();
    }
    window->_key_mods = mods;


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
    if (!has_focused_area && window->_key_callback != nullptr) {
        window->_key_callback(ptr, key, scancode, action, mods);
    }
}
CATCH_AND_RETHROW_FUNC_EXC;

// Character input callback
void Window::char_callback(GLFWwindow* ptr, unsigned int codepoint)
{
    Window* window = get(ptr);
    if (!window)
        return;
    Context const context = window->setContext();

    // Block inputs
    if (window->_block_inputs)
        return;

    std::u32string str(1, static_cast<char32_t>(codepoint));
    TR::Area::cursorAddText(str);

    // Call user defined callback, if needed
    if (window->_char_callback != nullptr) {
        window->_char_callback(ptr, codepoint);
    }
}

void Window::_retrieveMonitors()
{
    // Clear vector
    _main._monitors.clear();
    // Retrieve all monitors
    int size;
    GLFWmonitor** arr = glfwGetMonitors(&size);
    // Store pointers in our vector
    _main._monitors.resize(size);
    for (int i = 0; i < size; i++) {
        _main._monitors[i] = arr[i];
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

     _retrieveMonitors();
}
CATCH_AND_RETHROW_FUNC_EXC;

SSS_GL_END;