#pragma once

// OpenGL headers
#include <glad/glad.h>  // glad
#include <GLFW/glfw3.h> // glfw
#include <glm/glm.hpp>  // glm
#include <glm/ext.hpp>  // glm

// SSS libs
#include <SSS/Commons.hpp>
#include <SSS/Text-Rendering.hpp>

// STL
#include <array>
#include <fstream>
#include <algorithm>

#define __SSS_GL_BEGIN __SSS_BEGIN namespace GL {
#define __SSS_GL_END __SSS_END }

__SSS_GL_BEGIN

struct LOG {
    struct InternalCallbacks {
        static bool window_resize;
        static bool window_pos;
        static bool mouse_position;
        static bool mouse_button;
        static bool key;
        static bool monitor;
    };
};

__SSS_GL_END