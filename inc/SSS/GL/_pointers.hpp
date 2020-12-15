#pragma once

#include <SSS/Commons/pointers.hpp>
#include "SSS/GL/_includes.hpp"

__SSS_GL_BEGIN
__INTERNAL_BEGIN

using GLFWwindow_Ptr = C_Ptr
    <GLFWwindow, void(*)(GLFWwindow*), glfwDestroyWindow>;

__INTERNAL_END
__SSS_GL_END