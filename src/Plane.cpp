#include "SSS/GL/Plane.hpp"
#include "SSS/GL/Window.hpp"

__SSS_GL_BEGIN

Plane::Plane(std::weak_ptr<Window> window) try
    : Model(window)
{
    Context const context(_window);

    constexpr float vertices[] = {
        // positions          // texture coords (1 - y)
        -0.5f,  0.5f, 0.0f,   0.0f, 1.f - 1.0f,   // top left
        -0.5f, -0.5f, 0.0f,   0.0f, 1.f - 0.0f,   // bottom left
         0.5f, -0.5f, 0.0f,   1.0f, 1.f - 0.0f,   // bottom right
         0.5f,  0.5f, 0.0f,   1.0f, 1.f - 1.0f    // top right
    };
    constexpr unsigned int indices[] = {
        0, 1, 2,  // first triangle
        0, 2, 3   // second triangle
    };

    _vao->bind();
    _vbo->bind();
    _ibo->bind();

    _vbo->edit(sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    _ibo->edit(sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}
__CATCH_AND_RETHROW_METHOD_EXC

Plane::~Plane()
{
}

void Plane::useTexture(uint32_t texture_id)
{
    _texture_id = texture_id;
    _use_texture = true;
    _updateTexScaling();
}

glm::mat4 Plane::getModelMat4()
{
    if (_should_compute_mat4) {
        glm::mat4 scaling = glm::scale(_scaling, _tex_scaling);
        _model_mat4 = _translation * _rotation * scaling;
        _should_compute_mat4 = false;
    }
    return _model_mat4;
}

void Plane::draw() const try
{
    Window::Shared const window = _window.lock();
    if (!window) {
        return;
    }
    Context const context(_window);
    _vao->bind();
    // Bind texture if needed
    if (_use_texture) {
        window->getObjects().textures.at(_texture_id)->bind();
    }
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}
__CATCH_AND_RETHROW_METHOD_EXC

// Sets the function to be called when the button is clicked.
// The function MUST be of the format void (*)();
void Plane::setFunction(ButtonFunction func) try
{
    _f = func;
}
__CATCH_AND_RETHROW_METHOD_EXC

// Calls the function set via setFunction();
// Called whenever the button is clicked.
void Plane::callFunction() try
{
    if (!_use_as_button) {
        return;
    }
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

void Plane::_updateTexScaling()
{
    // Retrieve texture dimensions
    Window::Shared const window = _window.lock();
    if (!window || !_use_texture) {
        _tex_scaling = glm::vec3(1);
        _should_compute_mat4 = true;
        return;
    }
    // Check if dimensions changed
    int w, h;
    window->getObjects().textures.at(_texture_id)->getDimensions(w, h);
    if (_tex_w == w && _tex_h == h) {
        return;
    }
    // Update dimensions
    _tex_w = w;
    _tex_h = h;

    // Get texture ratio
    float const ratio = (static_cast<float>(_tex_w) / static_cast<float>(_tex_h));
    // Calculate according scaling
    glm::vec3 scaling(1);
    if (ratio < 1.f) {
        scaling[1] = 1 / ratio;
    }
    else {
        scaling[0] = ratio;
    }

    // If scaling changed, indicate that the Model matrice should be computed
    if (_tex_scaling != scaling) {
        _tex_scaling = scaling;
        _should_compute_mat4 = true;
    }
}

bool Plane::_hoverTriangle(glm::vec3 const& A, glm::vec3 const& B,
    glm::vec3 const& C, glm::vec3 const& P, bool is_abc)
{
    // Skip if one (or more) of the points is behind the camera
    if (A.z > 1.f || B.z > 1.f || C.z > 1.f) {
        return false;
    }

    // Check if P is inside the triangle ABC via barycentric coordinates
    float const denominator = ((B.y - C.y) * (A.x - C.x) + (C.x - B.x) * (A.y - C.y));
    float const a = ((B.y - C.y) * (P.x - C.x) + (C.x - B.x) * (P.y - C.y)) / denominator;
    float const b = ((C.y - A.y) * (P.x - C.x) + (A.x - C.x) * (P.y - C.y)) / denominator;
    float const c = 1.f - a - b;
    if (!(0.f <= a && a <= 1.f && 0.f <= b && b <= 1.f && 0.f <= c && c <= 1.f)) {
        return false;
    }

    // End function if no texture provided or if the Plane isn't drawn as a rectangle
    if (!_use_texture || !(A.x == B.x && B.y == C.y)) {
        _is_hovered = true;
        return true;
    }

    // Get the relative x & y position the cursor is hovering over.
    // Inverse x & y based on if the triangle is ABC or CDA
    _relative_x = static_cast<int>(c * static_cast<float>(_tex_w));
    _relative_y = static_cast<int>(a * static_cast<float>(_tex_h));
    if (is_abc) {
        _relative_y = _tex_h - _relative_y;
    }
    else {
        _relative_x = _tex_w - _relative_x;
    }

    // Retrieve window, ensure it exists. Then retrieve the texture
    // and ensure it exists too. Skip if the texture is Text.
    Window::Shared const window = _window.lock();
    if (!window) {
        return true;
    }
    Texture::Ptr const& texture = window->getObjects().textures.at(_texture_id);
    if (!texture) {
        return true;
    }
    if (texture->getType() == Texture::Type::Text) {
        _is_hovered = true;
        return true;
    }

    // Update status if the position is inside the texture
    size_t const pixel = static_cast<size_t>(_relative_y * _tex_w + _relative_x);
    if (pixel < texture->_pixels.size()) {
        _is_hovered = texture->_pixels.at(pixel).bytes.a != 0;
    }

    return true;
}

// Updates _is_hovered via the mouse position callback.
void Plane::_updateHoverStatus(double x, double y) try
{
    // Skip if not used as a button
    if (!_use_as_button) {
        return;
    }

    // Reset hover status
    _is_hovered = false;

    // Plane MVP matrix
    glm::mat4 const mvp = getMVP();
    // Plane's coordinates
    glm::vec4 const A4 = mvp * glm::vec4(-0.5, 0.5, 0, 1);   // Top left
    glm::vec4 const B4 = mvp * glm::vec4(-0.5, -0.5, 0, 1);  // Bottom left
    glm::vec4 const C4 = mvp * glm::vec4(0.5, -0.5, 0, 1);   // Bottom right
    glm::vec4 const D4 = mvp * glm::vec4(0.5, 0.5, 0, 1);    // Top right
    // Normalize in screen coordinates
    glm::vec3 const A = A4 / A4.w;
    glm::vec3 const B = B4 / B4.w;
    glm::vec3 const C = C4 / C4.w;
    glm::vec3 const D = D4 / D4.w;
    // Mouse coordinates
    glm::vec3 const P(x, y, 0);

    if (_hoverTriangle(A, B, C, P, true)) {
        return;
    }
    _hoverTriangle(C, D, A, P, false);
}
__CATCH_AND_RETHROW_METHOD_EXC;

__SSS_GL_END
