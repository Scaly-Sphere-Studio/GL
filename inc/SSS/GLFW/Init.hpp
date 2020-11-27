#pragma once

#include "SSS/GLFW/_includes.hpp"

__SSS_GLFW_BEGIN

// On the first call : inits GLFW and sets TR's screen DPI.
// All further calls are ignored.
// Terminates GLFW automatically when exiting programm.
void init();

__INTERNAL_BEGIN
// Inits GLFW in constructor.
// Quits GLFW in destructor.
// Sets the Text Rendering's screen DPI (see TR::Font::setDPI;).
class Init {
public:
    Init();
    ~Init();
};
__INTERNAL_END

__SSS_GLFW_END