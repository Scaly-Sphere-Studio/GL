#pragma once

#include "_includes.hpp"

__SSS_GL_BEGIN;

/** \cond TODO*/
namespace LOG {
    struct internal_callbacks {
        static bool window_resize;
        static bool window_pos;
        static bool mouse_position;
        static bool mouse_button;
        static bool key;
        static bool monitor;
    };
};
/** \endcond*/

__INTERNAL_BEGIN;

void window_iconify_callback(GLFWwindow* ptr, int state);
// Resizes the internal width and height of correspondig Window instance
void window_resize_callback(GLFWwindow* ptr, int w, int h);
// Determines current monitor of the window
void window_pos_callback(GLFWwindow* ptr, int x, int y);
// Used for clickable buttons and such
void mouse_button_callback(GLFWwindow* ptr, int button, int action, int mods);
void mouse_position_callback(GLFWwindow* ptr, double x, double y);
// Stores key inputs
void key_callback(GLFWwindow* ptr, int key, int scancode, int action, int mods);

// Updates connected monitors
void monitor_callback(GLFWmonitor* ptr, int event);

__INTERNAL_END;
__SSS_GL_END;