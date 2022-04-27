#pragma once

#include "Model.hpp"

/** @file
 *  Defines class SSS::GL::Plane.
 */

SSS_GL_BEGIN;

INTERNAL_BEGIN;
void mouse_button_callback(GLFWwindow*, int, int, int); // Pre-declaration
class PlaneRenderer;
INTERNAL_END;

class Plane : public Model {
    friend void _internal::mouse_button_callback(GLFWwindow*, int, int, int);
    friend class _internal::PlaneRenderer;
    friend class Window;
    friend class Texture;

protected:
    Plane(std::weak_ptr<Window> window);

public:
    virtual ~Plane();

    using Ptr = std::unique_ptr<Plane>;
    using Renderer = _internal::PlaneRenderer;
    
    void setTextureID(uint32_t texture_id);
    inline uint32_t getTextureID() const noexcept { return _texture_id; };

    virtual glm::mat4 getModelMat4();
    virtual void getAllTransformations(glm::vec3& scaling, glm::vec3& rot_angles,
        glm::vec3& translation);
    inline glm::vec3 getTexScaling() const noexcept { return _tex_scaling; };

    // Hitbox used to trigger 'on click' events
    enum class Hitbox {
        None,   // No hitbox.
        Alpha,  // Hitbox if alpha > 0.
        Full    // Hitbox if within plane coordinates.
    };
    inline void setHitbox(Hitbox hitbox) noexcept { _hitbox = hitbox; };
    inline Hitbox getHitbox() const noexcept { return _hitbox; };

protected:
    uint32_t _texture_id{ 0 };
    bool _use_texture{ false };
    GLsizei _tex_w{ 0 }, _tex_h{ 0 };
    glm::vec3 _tex_scaling{ 1 };

    void _updateTexScaling();

    // Type of hitbox
    Hitbox _hitbox{ Hitbox::None };

    // Mouse hovering relative position, updated via Window::render every x ms.
    int _relative_x{ 0 };
    int _relative_y{ 0 };

    // Called from _isHovered
    bool _hoverTriangle(glm::mat4 const& mvp, glm::vec4 const& A,
        glm::vec4 const& B, glm::vec4 const& C, float x, float y,
        double &z, bool& is_hovered);
    // Returns true and updates z if Plane is hovered
    bool _isHovered(glm::mat4 const& VP, float x, float y, double &z);
};

SSS_GL_END;