#pragma once

#include "_includes.hpp"

/** @file
 *  Defines internal callback functions (doesn't affect user callbacks).
 */

namespace SSS::Log::GL {
    /** Logging properties for internal SSS::GL::Window callbacks.*/
    struct Callbacks : public LogBase<Callbacks> {
        using LOG_STRUCT_BASICS(Log, Callbacks);
        bool window_resize = false;
        bool window_pos = false;
        bool window_iconify = false;
        bool mouse_position = false;
        bool mouse_button = false;
        bool key = false;
        bool monitor = false;
    };
};

SSS_GL_BEGIN;
INTERNAL_BEGIN;

// Resizes the internal width and height of correspondig Window instance
void window_resize_callback(GLFWwindow* ptr, int w, int h);
// Determines current monitor of the window
void window_pos_callback(GLFWwindow* ptr, int x, int y);
void window_iconify_callback(GLFWwindow* ptr, int state);
void mouse_position_callback(GLFWwindow* ptr, double x, double y);
// Used for clickable buttons and such
void mouse_button_callback(GLFWwindow* ptr, int button, int action, int mods);
// Stores key inputs
void key_callback(GLFWwindow* ptr, int key, int scancode, int action, int mods);

// Updates connected monitors
void monitor_callback(GLFWmonitor* ptr, int event);

INTERNAL_END;
SSS_GL_END;