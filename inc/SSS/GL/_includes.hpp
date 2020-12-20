#pragma once

// OpenGL headers
#include <GL/glew.h>    // glew
#include <GLFW/glfw3.h> // glfw
#include <glm/glm.hpp>  // glm
#include <glm/ext.hpp>  // glm

// SSS libs
#include <SSS/Commons.hpp>
#include <SSS/Text-Rendering.hpp>

// STL
#include <fstream>
#include <algorithm>

#define __SSS_GL_BEGIN __SSS_BEGIN namespace GL {
#define __SSS_GL_END __SSS_END }