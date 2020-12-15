#pragma once

// GLEW & GLFW
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// SSS libs
#include <SSS/Commons.hpp>
#include <SSS/Text-Rendering.hpp>

// STL
#include <fstream>
#include <algorithm>

#define __SSS_GLFW_BEGIN __SSS_BEGIN namespace GLFW {
#define __SSS_GLFW_END __SSS_END }