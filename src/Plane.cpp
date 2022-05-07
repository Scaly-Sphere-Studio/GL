#include "SSS/GL/Plane.hpp"
#include "SSS/GL/Window.hpp"
#include "SSS/GL/Texture.hpp"

SSS_GL_BEGIN;

Plane::Plane(std::weak_ptr<Window> window) try
    : Model(window)
{
}
CATCH_AND_RETHROW_METHOD_EXC;

Plane::~Plane()
{
}

void Plane::setTextureID(uint32_t texture_id)
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

void Plane::getAllTransformations(glm::vec3& scaling, glm::vec3& rot_angles, glm::vec3& translation)
{
    Model::getAllTransformations(scaling, rot_angles, translation);
    scaling /= _tex_scaling;
}

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

bool Plane::_hoverTriangle(glm::mat4 const& mvp, glm::vec4 const& A,
    glm::vec4 const& B, glm::vec4 const& C, float x, float y,
    double& z, bool& is_hovered)
{
    // Skip if one (or more) of the points is behind the camera
    if (A.z > 1.f || B.z > 1.f || C.z > 1.f) {
        return false;
    }

    // Check if P is inside the triangle ABC via barycentric coordinates
    float const denominator = ((B.y - C.y) * (A.x - C.x) + (C.x - B.x) * (A.y - C.y));
    float const a = ((B.y - C.y) * (x - C.x) + (C.x - B.x) * (y - C.y)) / denominator;
    float const b = ((C.y - A.y) * (x - C.x) + (A.x - C.x) * (y - C.y)) / denominator;
    float const c = 1.f - a - b;
    if (!(0.f <= a && a <= 1.f && 0.f <= b && b <= 1.f && 0.f <= c && c <= 1.f)) {
        return false;
    }
    // Compute relative Z (not the real scene Z, as it's computed via
    // normalized vertices)
    z = a * A.z + b * B.z + c * C.z;
    // If z < -1.f, then it is behind the camera
    if (z < -1.f) {
        return true;
    }

    // Inverse and normalize relative x/y in a -0.5/+0.5 range
    glm::vec4 P = glm::inverse(mvp) * glm::vec4(x, y, z, 1);
    P /= P.w;
    // Convert x/y in a 0/1 range, and inverse y (openGL y VS texture y)
    P.x += 0.5f;
    P.y += 0.5f;
    P.y = 1.f - P.y;
    // Get the relative x & y position the cursor is hovering over.
    _relative_x = static_cast<int>(P.x * static_cast<float>(_tex_w));
    _relative_y = static_cast<int>(P.y * static_cast<float>(_tex_h));

    // End function if no texture provided, or if the hitbox doesn't care about alpha.
    if (!_use_texture || _hitbox == Hitbox::Full) {
        is_hovered = true;
        return true;
    }

    // Retrieve Texture.
    Window::Shared const window = _window.lock();
    if (!window) {
        return true;
    }
    Window::Objects const& objects = window->getObjects();
    if (objects.textures.count(_texture_id) == 0) {
        return true;
    }
    Texture::Ptr const& texture = objects.textures.at(_texture_id);
    if (!texture) {
        return true;
    }

    // Update status if the position is on an opaque pixel
    size_t const pixel = static_cast<size_t>(_relative_y * _tex_w + _relative_x);
    switch (texture->getType()) {
    case Texture::Type::Raw:
        {
            if (pixel < texture->getRawPixels().size()) {
                is_hovered = texture->getRawPixels().at(pixel).bytes.a != 0;
            }
        }
        break;
    case Texture::Type::Text:
        {
            TR::Area::Ptr const& text_area = texture->getTextArea();
            if (text_area) {
                int w, h;
                text_area->getDimensions(w, h);
                size_t const size = static_cast<size_t>(w) * static_cast<size_t>(h);
                if (pixel < size) {
                    void const* pixels = text_area->pixelsGet();
                    is_hovered = static_cast<RGBA32 const*>(pixels)[pixel].bytes.a != 0;
                }
            }
        }
        break;
    }

    return true;
}

// Updates _is_hovered via the mouse position callback.
bool Plane::_isHovered(glm::mat4 const& VP, float x, float y, double &z) try
{
    // Skip if no hitbox
    if (_hitbox == Hitbox::None) {
        return false;
    }

    // Plane MVP matrix
    glm::mat4 const mvp = VP * getModelMat4();
    // Plane's coordinates
    glm::vec4 const A4 = mvp * glm::vec4(-0.5, 0.5, 0, 1);   // Top left
    glm::vec4 const B4 = mvp * glm::vec4(-0.5, -0.5, 0, 1);  // Bottom left
    glm::vec4 const C4 = mvp * glm::vec4(0.5, -0.5, 0, 1);   // Bottom right
    glm::vec4 const D4 = mvp * glm::vec4(0.5, 0.5, 0, 1);    // Top right
    // Normalize in screen coordinates
    glm::vec4 const A = A4 / A4.w;
    glm::vec4 const B = B4 / B4.w;
    glm::vec4 const C = C4 / C4.w;
    glm::vec4 const D = D4 / D4.w;

    // Test for ABC or CDA
    bool is_hovered = false;
    _hoverTriangle(mvp, A, B, C, x, y, z, is_hovered)
        || _hoverTriangle(mvp, C, D, A, x, y, z, is_hovered);
    return is_hovered;
}
CATCH_AND_RETHROW_METHOD_EXC;

SSS_GL_END;