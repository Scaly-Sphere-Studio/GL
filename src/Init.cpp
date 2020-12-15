#include "SSS/GL/Init.hpp"

__SSS_GL_BEGIN

// On the first call : inits GLFW and sets TR's screen DPI.
// All further calls are ignored.
// Terminates GLFW automatically when exiting programm.
void init()
{
    static const _internal::Init init;
}

__INTERNAL_BEGIN

// Inits GLFW in constructor.
// Quits GLFW in destructor.
// Sets the Text Rendering's screen DPI (see TR::Font::setDPI;).
Init::Init()
{
    // Init GLFW
    glfwInit();
    __LOG_MSG("GLFW initialized.");
    // Retrive monitors
    monitor_callback(nullptr, 0);
    glfwSetMonitorCallback(monitor_callback);
}

Init::~Init()
{
    // Terminate GLFW
    glfwTerminate();
    __LOG_MSG("GLFW terminated.");
};
__INTERNAL_END

__SSS_GL_END