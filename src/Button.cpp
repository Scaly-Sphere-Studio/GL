#include "SSS/GL/Button.hpp"

__SSS_GL_BEGIN

Button::Button(Texture2D::Shared texture, GLFWwindow const* context) try
    : Plane::Plane(texture, context)
{
}
__CATCH_AND_RETHROW_METHOD_EXC

// Sets the function to be called when the button is clicked.
// The function MUST be of the format void (*)();
void Button::setFunction(ButtonFunction func) try
{
    _f = func;
}
__CATCH_AND_RETHROW_METHOD_EXC

// Calls the function set via setFunction();
// Called whenever the button is clicked.
void Button::callFunction() try
{
    if (_f == nullptr) {
        __LOG_METHOD_WRN("Function wasn't set.");
        return;
    }
    _f();
}
__CATCH_AND_RETHROW_METHOD_EXC

// Updates _is_hovered via the mouse position callback.
void Button::_updateHoverStatus(double x, double y) try
{
    // Reset hover status
    _is_hovered = false;

    // Get the button's relative position using its matrice
    glm::vec2 const u = getModelMat4() * glm::vec4(-0.5, -0.5, 0, 1);
    glm::vec2 const v = getModelMat4() * glm::vec4(0.5, 0.5, 0, 1);
    // Check if the mouse is inside the rectangle
    if (!(x > u[0] && x < v[0] && y > u[1] && y < v[1])) {
        return;
    }

    // If the button is a PNG, check the alpha channel of the pixel being hovered.
    RGBA32::Pixels const& pixels = _texture->getPixels();
    if (pixels.empty()) {
        _is_hovered = true;
    }
    else {
        float const x_range = v[0] - u[0],
            y_range = v[1] - u[1],
            x_diff = static_cast<float>(x) - u[0],
            y_diff = static_cast<float>(y) - u[1];
        int const x_pos = static_cast<int>(x_diff / x_range * static_cast<float>(_tex_w)),
            y_pos = _tex_h - static_cast<int>(y_diff / y_range * static_cast<float>(_tex_h));
        // Update status
        size_t const pixel = static_cast<size_t>(y_pos * _tex_w + x_pos);
        if (pixel < pixels.size()) {
            _is_hovered = pixels.at(pixel).bytes.a != 0;
        }
    }
}
__CATCH_AND_RETHROW_METHOD_EXC

__SSS_GL_END