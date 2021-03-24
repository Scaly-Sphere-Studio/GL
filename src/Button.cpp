#include "SSS/GL/Button.hpp"

__SSS_GL_BEGIN

Button::Button()
    : Plane::Plane()
{
}

Button::Button(std::string const& filepath)
    : Plane::Plane(filepath)
{
}

void Button::_updateHoverStatus(double x, double y)
{
    // Reset hover status
    _is_hovered = false;

    // Get the button's relative position using its matrice
    glm::vec2 const u = getModelMat4() * glm::vec4(-0.5, -0.5, 0, 1);
    glm::vec2 const v = getModelMat4() * glm::vec4(0.5, 0.5, 0, 1);
    if (!(x > u[0] && x < v[0] && y > u[1] && y < v[1])) {
        return;
    }

    // If the button is a PNG, check the alpha channel of the pixel being hovered.
    if (!_texture_alpha_map.empty()) {
        float const x_range = v[0] - u[0],
            y_range = v[1] - u[1],
            x_diff = static_cast<float>(x) - u[0],
            y_diff = static_cast<float>(y) - u[1];
        int const x_pos = static_cast<int>(x_diff / x_range * static_cast<float>(_tex_w)),
            y_pos = _tex_h - static_cast<int>(y_diff / y_range * static_cast<float>(_tex_h));
        // Update status
        _is_hovered = _texture_alpha_map.at(y_pos * _tex_w + x_pos);
    }
}

__SSS_GL_END