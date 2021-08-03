#pragma once

// OpenGL headers
#include <glad/glad.h>              // glad
#include <GLFW/glfw3.h>             // glfw
#include <glm/glm.hpp>              // glm
#include <glm/ext.hpp>              // glm
#include <glm/gtc/quaternion.hpp>   // glm
#include <glm/gtx/quaternion.hpp>   // glm

// SSS libs
#include <SSS/Commons.hpp>
#include <SSS/Text-Rendering.hpp>

// STL
#include <array>
#include <fstream>
#include <mutex>
#include <shared_mutex>
#include <algorithm>

#define __SSS_GL_BEGIN __SSS_BEGIN namespace GL {
#define __SSS_GL_END __SSS_END }