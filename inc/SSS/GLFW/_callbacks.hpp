#pragma once

#include "SSS/GLFW/_includes.hpp"

__SSS_GLFW_BEGIN
__INTERNAL_BEGIN

// Resizes the internal width and height of correspondig Window instance
void window_resize_callback(GLFWwindow* ptr, int w, int h);
// Determines current monitor of the window
void window_pos_callback(GLFWwindow* ptr, int x, int y);

// Updates connected monitors
void monitor_callback(GLFWmonitor* ptr, int event);

__INTERNAL_END
__SSS_GLFW_END