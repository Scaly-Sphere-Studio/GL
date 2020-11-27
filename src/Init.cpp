#include "SSS/GLFW/Init.hpp"

__SSS_GLFW_BEGIN

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

    // Retrieve screen dimensions (in millimeters)
    int width_mm, height_mm;
    glfwGetMonitorPhysicalSize(glfwGetPrimaryMonitor(), &width_mm, &height_mm);
    // Retrieve screen resolution (in pixels)
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    // Constant to convert millimeters into inches
    constexpr float mm_to_inches = 0.0393701f;
    // Horizontal DPI
    int const hdpi = std::lround(
        static_cast<float>(mode->width)
        / (static_cast<float>(width_mm) * mm_to_inches)
    );
    // Vertical DPI
    int const vdpi = std::lround(
        static_cast<float>(mode->height)
        / (static_cast<float>(height_mm) * mm_to_inches)
    );

    // Set TR's DPIs
    TR::Font::setDPI(FT_UInt(hdpi), FT_UInt(vdpi));
    __LOG_MSG(context_msg("Text rendering DPI set", toString(hdpi)) + "x" + toString(vdpi));
}

Init::~Init()
{
    // Terminate GLFW
    glfwTerminate();
    __LOG_MSG("GLFW terminated.");
};
__INTERNAL_END

__SSS_GLFW_END