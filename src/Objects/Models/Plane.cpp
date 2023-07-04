#include "GL/Objects/Models/Plane.hpp"

SSS_GL_BEGIN;

Plane::Plane() try
{
}
CATCH_AND_RETHROW_METHOD_EXC;

Plane::Shared Plane::create(Texture::Shared texture)
{
    Shared ret = create();
    ret->setTexture(texture);
    return ret;
}

Plane::Shared Plane::duplicate() const
{
    Shared plane = create();
    *plane = *this;
    return plane;
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

void Plane::setTexture(Texture::Shared texture)
{
    _texture = texture;
    _updateTexScaling();
}

void Plane::_updateTextureOffset()
{
    _texture_offset = 0;
    if (!_texture || _texture->getTotalFramesTime() == std::chrono::nanoseconds(0))
        return;
    auto const& frames = _texture->getFrames();

    // If loop disabled & animation completed, stop playing
    if (!_looping && _animation_duration >= _texture->getTotalFramesTime()) {
        _is_playing = false;
        _animation_duration = std::chrono::nanoseconds(0);
        _texture_offset = static_cast<uint32_t>(frames.size()) - 1;
    }

    // Remove excess time
    while (_animation_duration >= _texture->getTotalFramesTime()) {
        _animation_duration -= _texture->getTotalFramesTime();
    }

    // Find current texture offset
    auto duration = _animation_duration;
    for (uint32_t i = 0; i < frames.size(); ++i) {
        duration -= frames[i].delay;
        if (duration < std::chrono::nanoseconds(0)) {
            _texture_offset = i;
            break;
        }
    }
}

void Plane::_updateTexScaling()
{
    if (!_texture) {
        _tex_scaling = glm::vec3(1);
        _should_compute_mat4 = true;
        return;
    }
    // Check if dimensions changed
    int w, h;
    _texture->getCurrentDimensions(w, h);
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

bool Plane::_hoverTriangle(glm::mat4 const& mvp, glm::vec3 const& A,
    glm::vec3 const& B, glm::vec3 const& C, double x, double y,
    double& z, bool& is_hovered)
{
    // Skip if one (or more) of the points is behind the camera
    if (A.z > 1.f || B.z > 1.f || C.z > 1.f) {
        return false;
    }

    // Check if P is inside the triangle ABC via barycentric coordinates
    float const denominator = ((B.y - C.y) * (A.x - C.x) + (C.x - B.x) * (A.y - C.y));
    float const a = ((B.y - C.y) * static_cast<float>(x - C.x)
                  +  (C.x - B.x) * static_cast<float>(y - C.y))
                  / denominator;
    float const b = ((C.y - A.y) * static_cast<float>(x - C.x)
                  +  (A.x - C.x) * static_cast<float>(y - C.y))
                  / denominator;
    float const c = 1.f - a - b;
    // Compute relative Z (not the real scene Z, as it's computed via
    // normalized vertices)
    z = static_cast<double>(a * A.z + b * B.z + c * C.z);

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

    // If z < -1.f, then it is behind the camera
    if (z < -1.f) {
        return true;
    }
    // In or outside the triangle
    if (!(0.f <= a && a <= 1.f && 0.f <= b && b <= 1.f && 0.f <= c && c <= 1.f)) {
        return false;
    }

    // End function if no texture provided, or if the hitbox doesn't care about alpha.
    if (!_texture || _hitbox == Hitbox::Full) {
        is_hovered = true;
        return true;
    }

    // Update status if the position is on an opaque pixel
    size_t const pixel = static_cast<size_t>(_relative_y * _tex_w + _relative_x);
    switch (_texture->getType()) {
    case Texture::Type::Raw: {
        if (pixel < _texture->getRawPixels(_texture_offset).size()) {
            is_hovered = _texture->getRawPixels().at(pixel).a != 0;
        }
        break;
    }
    case Texture::Type::Text: {
        TR::Area* text_area = _texture->getTextArea();
        if (text_area) {
            int w, h;
            text_area->pixelsGetDimensions(w, h);
            size_t const size = static_cast<size_t>(w) * static_cast<size_t>(h);
            if (pixel < size) {
                void const* pixels = text_area->pixelsGet();
                is_hovered = static_cast<RGBA32 const*>(pixels)[pixel].a != 0;
            }
        }
        break;
    }
    default:
        break;
    }

    return true;
}

// Updates _is_hovered via the mouse position callback.
bool Plane::_isHovered(glm::mat4 const& VP, double x, double y, double &z) try
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
    glm::vec3 const A = A4 / A4.w;
    glm::vec3 const B = B4 / B4.w;
    glm::vec3 const C = C4 / C4.w;
    glm::vec3 const D = D4 / D4.w;

    // Test for ABC or CDA
    bool is_hovered = false;
    _hoverTriangle(mvp, A, B, C, x, y, z, is_hovered)
        || _hoverTriangle(mvp, C, D, A, x, y, z, is_hovered);
    return is_hovered;
}
CATCH_AND_RETHROW_METHOD_EXC;

SSS_GL_END;