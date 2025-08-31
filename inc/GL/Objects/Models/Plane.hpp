#ifndef SSS_GL_PLANE_HPP
#define SSS_GL_PLANE_HPP

#include "../Model.hpp"
#include "../Texture.hpp"
#include <SSS/Commons/eventList.hpp>
#include <set>

/** @file
 *  Defines class SSS::GL::Plane.
 */

SSS_GL_BEGIN;

// Ignore warning about STL exports as they're private members
#pragma warning(push, 2)
#pragma warning(disable: 4251)
#pragma warning(disable: 4275)

/** 2D plane derived from Model.*/
class SSS_GL_API PlaneBase : public Observer, public ModelBase {
    friend class PlaneRenderer;
    friend class Window;
    friend SSS_GL_API void pollEverything();

private:
    static std::set<std::reference_wrapper<PlaneBase>> _instances;

protected:
    PlaneBase();

    // Make copy & move operations private
    PlaneBase(const PlaneBase&)             = delete;   // Copy constructor
    PlaneBase(PlaneBase&&)                  = delete;   // Move constructor
    PlaneBase& operator=(const PlaneBase&)  = default;  // Copy assignment
    PlaneBase& operator=(PlaneBase&&)       = delete;   // Move assignment

public:
    /** Destructor, default.*/
    virtual ~PlaneBase();

protected:
    virtual glm::mat4 _getScalingMat4() const override;
public:
    virtual void getAllTransformations(glm::vec3& scaling, glm::vec3& rot_angles,
        glm::vec3& translation) const override;

    /** Sets the Texture to be used for this instance.*/
    void setTexture(Texture::Shared texture);;
    /** Returns the Texture used for this instance.*/
    inline Texture::Shared getTexture() const noexcept { return _texture; };
    inline TR::Area::Shared getTextArea() const noexcept { return _texture ? _texture->getTextArea() : nullptr; };

    void setTextureCallback(std::function<void(PlaneBase&)> func) { _texture_callback = func; };
    void setTextureSizeCallback(std::function<void(PlaneBase&)> func) { _texture_size_callback = func; };

    inline void play() noexcept { _is_playing = true; };
    inline void pause() noexcept { _is_playing = false; };
    inline void stop() noexcept { _is_playing = false; _animation_duration = std::chrono::nanoseconds(0); };

    inline bool isPlaying() const noexcept { return _is_playing; };
    inline bool isPaused() const noexcept { return !_is_playing && _animation_duration != std::chrono::nanoseconds(0); };
    inline bool isStopped() const noexcept { return !_is_playing && _animation_duration == std::chrono::nanoseconds(0); };

    void setLooping(bool enable) noexcept { _looping = enable; };
    bool isLooping() const noexcept { return _looping; };

    void setAlpha(float alpha) noexcept;
    inline float getAlpha(void) const noexcept { return _alpha; };

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

    /** Returns relative hovering coordinates (only valid if isHovered() returns true).*/
    inline void getRelativeCoords(int& x, int& y) const noexcept { x = _relative_x; y = _relative_y; };

    inline uint32_t getTexOffset() const noexcept { return _texture_offset; };

private:
    void _setTextureOffset(uint32_t offset);
    void _updateTextureOffset();

    Texture::Shared _texture;
    std::function<void(PlaneBase&)> _texture_callback;
    std::function<void(PlaneBase&)> _texture_size_callback;
    uint32_t _texture_offset{ 0 };
    bool _is_playing{ false };
    bool _looping{ false };
    std::chrono::nanoseconds _animation_duration{ 0 };
    float _alpha{ 1.f };
    GLsizei _tex_w{ 0 }, _tex_h{ 0 };
    glm::vec3 _tex_scaling{ 1 };

    void _updateTexScaling();

    virtual void _subjectUpdate(Subject const& subject, int event_id) override;

    // Type of hitbox
    Hitbox _hitbox{ Hitbox::None };

    // Mouse hovering relative position, updated via Window::render every x ms.
    int _relative_x{ 0 };
    int _relative_y{ 0 };

    // Called from _isHovered
    bool _hoverTriangle(glm::mat4 const& mvp, glm::vec3 const& A,
        glm::vec3 const& B, glm::vec3 const& C, double x, double y,
        double &z, bool& is_hovered);
    // Returns true and updates z if Plane is hovered
    bool _isHovered(glm::mat4 const& VP, double x, double y, double &z);
};

template<class Derived>
class PlaneTemplate : public PlaneBase, public InstancedClass<Derived> {
public:
    using InstancedClass<Derived>::create;

    static auto create(Texture::Shared texture)
    {
        auto ret = create();
        ret->setTexture(texture);
        return ret;
    }

    auto duplicate() const
    {
        auto shared = create();
        std::memcpy(shared.get(), this, sizeof(Derived));
        return shared;
    }
};

class Plane : public PlaneTemplate<Plane> {
    friend class SharedClass;
private:
    Plane() = default;
};

#pragma warning(pop)

SSS_GL_END;

#endif // SSS_GL_PLANE_HPP