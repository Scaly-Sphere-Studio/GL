#pragma once

#include "SSS/GL/Objects/Model.hpp"

/** @file
 *  Defines class SSS::GL::Plane.
 */

SSS_GL_BEGIN;

/** 2D plane derived from Model.*/
class Plane final : public Model<Plane> {
    friend class PlaneRenderer;
    friend class Window;
    friend class Texture;

private:
    Plane(std::weak_ptr<Window> window);

    // Make copy & move operations private
    Plane(const Plane&)             = default;   // Copy constructor
    Plane(Plane&&)                  = default;   // Move constructor
    Plane& operator=(const Plane&)  = default;   // Copy assignment
    Plane& operator=(Plane&&)       = default;   // Move assignment

public:
    /** Destructor, default.*/
    virtual ~Plane();

    /** Shared ptr to Plane instance.*/
    using Shared = std::shared_ptr<Plane>;
    using Vector = std::vector<Shared>;
private:
    using Weak = std::weak_ptr<Plane>;
    static std::vector<Weak> _instances;
public:
    /** Creates a Shared plane instance.
     *  If no window is specified, the first one (Window::getFirst()) is used.
     */
    static Shared create(std::shared_ptr<Window> win = nullptr);
    Shared duplicate() const;

    static Vector getInstances(Window::Shared window = nullptr) noexcept;

    virtual glm::mat4 getModelMat4();
    virtual void getAllTransformations(glm::vec3& scaling, glm::vec3& rot_angles,
        glm::vec3& translation);
    
    /** Sets the Texture ID to be used for this instance.*/
    void setTextureID(uint32_t texture_id);
    /** Returns the Texture ID used for this instance.*/
    inline uint32_t getTextureID() const noexcept { return _texture_id; };

    /** Types of hitboxes for on-click events to proc (or not)
     *  when in %Plane coordinates.
     */
    enum class Hitbox {
        /** No hitbox, click events never proc.*/
        None,
        /** %Alpha hitbox, click events proc when clicking on
         *  \b (alpha > 0) parts of corresponding Texture.
         */
        Alpha,
        /** %Full hitbox, click events always proc.*/
        Full
    };
    /** Sets the Hitbox type of this instance (default: Hitbox::None)*/
    inline void setHitbox(Hitbox hitbox) noexcept { _hitbox = hitbox; };
    /** Returns the Hitbox type of this instance.*/
    inline Hitbox getHitbox() const noexcept { return _hitbox; };

    /** Returns the curerntly hovered Plane instance, or an empty ptr.
     *  @sa isHovered()
     */
    static Shared getHovered() noexcept;
    /** Returns whether this Plane instance is currently hovered.
     *  @sa getHovered(), getRelativeCoords()
     */
    inline bool isHovered() const noexcept { return _is_hovered; };
    /** Returns relative hovering coordinates (only valid if isHovered() returns true).*/
    inline void getRelativeCoords(int& x, int& y) const noexcept { x = _relative_x; y = _relative_y; };

private:
    uint32_t _texture_id{ 0 };
    bool _use_texture{ false };
    GLsizei _tex_w{ 0 }, _tex_h{ 0 };
    glm::vec3 _tex_scaling{ 1 };

    void _updateTexScaling();

    // Type of hitbox
    Hitbox _hitbox{ Hitbox::None };

    // Whether the Plane instance is hovered, handled by Renderer
    bool _is_hovered{ false };
    // Mouse hovering relative position, updated via Window::render every x ms.
    int _relative_x{ 0 };
    int _relative_y{ 0 };

    // Called from _isHovered
    bool _hoverTriangle(glm::mat4 const& mvp, glm::vec3 const& A,
        glm::vec3 const& B, glm::vec3 const& C, float x, float y,
        double &z, bool& is_hovered);
    // Returns true and updates z if Plane is hovered
    bool _isHovered(glm::mat4 const& VP, float x, float y, double &z);
};

SSS_GL_END;