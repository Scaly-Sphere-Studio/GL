#ifndef SSS_GL_PLANE_HPP
#define SSS_GL_PLANE_HPP

#include "SSS/GL/Objects/Model.hpp"
#include "SSS/GL/Objects/Texture.hpp"

/** @file
 *  Defines class SSS::GL::Plane.
 */

SSS_GL_BEGIN;

/** 2D plane derived from Model.*/
class Plane final : public Model<Plane> {
    friend class _internal::SharedWindowObject<Plane>;
    friend class PlaneRenderer;
    friend class Window;
    friend class Texture;
    friend bool pollEverything();

private:
    Plane(std::shared_ptr<Window> window);

    // Make copy & move operations private
    Plane(const Plane&)             = default;   // Copy constructor
    Plane(Plane&&)                  = default;   // Move constructor
    Plane& operator=(const Plane&)  = default;   // Copy assignment
    Plane& operator=(Plane&&)       = default;   // Move assignment

public:
    /** Destructor, default.*/
    virtual ~Plane();

    using Model::create;
    static Shared create(Texture::Shared texture);
    Shared duplicate() const;

    virtual glm::mat4 getModelMat4();
    virtual void getAllTransformations(glm::vec3& scaling, glm::vec3& rot_angles,
        glm::vec3& translation);
    
    /** Sets the Texture to be used for this instance.*/
    void setTexture(Texture::Shared texture);;
    /** Returns the Texture used for this instance.*/
    inline Texture::Shared getTexture() const noexcept { return _texture; };

    inline void play() noexcept { _is_playing = true; };
    inline void pause() noexcept { _is_playing = false; };
    inline void stop() noexcept { _is_playing = false; _animation_duration = std::chrono::nanoseconds(0); };

    inline bool isPlaying() const noexcept { return _is_playing; };
    inline bool isPaused() const noexcept { return !_is_playing && _animation_duration != std::chrono::nanoseconds(0); };
    inline bool isStopped() const noexcept { return !_is_playing && _animation_duration == std::chrono::nanoseconds(0); };

    void setLooping(bool enable) noexcept { _looping = enable; };
    bool isLooping() const noexcept { return _looping; };

    inline void setAlpha(float alpha) noexcept { _alpha = std::clamp(alpha, 0.f, 1.f); };
    inline float getAlpha() const noexcept { return _alpha; };

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

    /** Returns the curerntly hovered Plane instance on given window,
     *  or an empty ptr.
     *  @sa isHovered()
     */
    static Shared getHovered(std::shared_ptr<Window> window = nullptr) noexcept;
    /** Returns whether this Plane instance is currently hovered.
     *  @sa getHovered(), getRelativeCoords()
     */
    inline bool isHovered() const noexcept { return _is_hovered; };
    /** Returns relative hovering coordinates (only valid if isHovered() returns true).*/
    inline void getRelativeCoords(int& x, int& y) const noexcept { x = _relative_x; y = _relative_y; };

private:
    void _updateTextureOffset();

    Texture::Shared _texture;
    uint32_t _texture_offset{ 0 };
    bool _is_playing{ false };
    bool _looping{ false };
    std::chrono::nanoseconds _animation_duration{ 0 };
    float _alpha{ 1.f };
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

#endif // SSS_GL_PLANE_HPP