#include "SSS/GL/Button.hpp"
#include "SSS/GL/Window.hpp"

__SSS_GL_BEGIN

Button::Button(std::weak_ptr<Window> window)
    : Plane(window)
{
}

Button::~Button()
{
}

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
    Window::Shared const window = _window.lock();
    if (window && _use_texture) {
        Texture::Ptr const& texture = window->getObjects().textures.at(_texture_id);
        if (texture->getType() == Texture::Type::Text) {
            texture->getTextArea()->placeCursor(_relative_x, _relative_y);
        }
    }
    if (_f != nullptr) {
        _f();
    }
}
__CATCH_AND_RETHROW_METHOD_EXC

glm::mat4 Button::getModelMat4() noexcept
{
    if (_should_compute_mat4) {
        glm::mat4 scaling = glm::scale(_scaling, _tex_scaling * _win_scaling);
        _model_mat4 = _translation * _rotation * scaling;
        _should_compute_mat4 = false;
    }
    return _model_mat4;
}

void Button::_updateWinScaling() try
{
    Window::Shared const window = _window.lock();
    if (!window) {
        return;
    }
    float const ratio = (static_cast<float>(_tex_w) / static_cast<float>(_tex_h));
    float const screen_ratio = window->getScreenRatio();
    glm::vec3 scaling(1);
    if (ratio < 1.f) {
        if (screen_ratio < ratio) {
            scaling[0] = 1 / ratio;
            scaling[1] = screen_ratio / ratio;
        }
        else {
            scaling[0] = 1 / screen_ratio;
        }
    }
    else {
        if (screen_ratio > ratio) {
            scaling[1] = ratio;
            scaling[0] = ratio / screen_ratio;
        }
        else {
            scaling[1] = screen_ratio;
        }
    }
    if (_win_scaling != scaling) {
        _win_scaling = scaling;
        _should_compute_mat4 = true;
    }
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
    // Else, set hovering as true.
    Window::Shared const window = _window.lock();
    if (!window || !_use_texture) {
        _is_hovered = true;
        return;
    }
    Texture::Ptr const& texture = window->getObjects().textures.at(_texture_id);
    if (texture->_type == Texture::Type::Text) {
        _is_hovered = true;
        return;
    }

    // Update relative x & y positions.
    float const x_range = v[0] - u[0],
        y_range = v[1] - u[1],
        x_diff = static_cast<float>(x) - u[0],
        y_diff = static_cast<float>(y) - u[1];
    _relative_x = static_cast<int>(x_diff / x_range * static_cast<float>(_tex_w));
    _relative_y = _tex_h - static_cast<int>(y_diff / y_range * static_cast<float>(_tex_h));

    // Update status
    size_t const pixel = static_cast<size_t>(_relative_y * _tex_w + _relative_x);
    if (pixel < texture->_pixels.size()) {
        _is_hovered = texture->_pixels.at(pixel).bytes.a != 0;
    }
}
__CATCH_AND_RETHROW_METHOD_EXC

__SSS_GL_END