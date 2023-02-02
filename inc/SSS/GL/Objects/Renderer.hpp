#ifndef SSS_GL_RENDERER_HPP
#define SSS_GL_RENDERER_HPP

#include "SSS/GL/Objects/Shaders.hpp"

/** @file
 *  Defines abstract class SSS::GL::Renderer.
 */

SSS_GL_BEGIN;

/** Abstract class for specialized rendering logic.
 *  Don't inherit from this, only used to be stored in Window instances
 *  @sa Window::createRenderer(), Plane::Renderer
 */
class RendererBase {
public:
    using Shared = std::shared_ptr<RendererBase>;
    using Vector = std::vector<Shared>;

    /** Optional title for UI purpose only.*/
    std::string title;

    /** Your rendering logic here.*/
    virtual void render() = 0;

private:
    Shaders::Shared _shaders;
public:
    /** Sets the Shaders to be used in render().*/
    inline void setShaders(Shaders::Shared shaders) noexcept { _shaders = shaders; };
    /** Retrieves the set Shaders to be used in render().*/
    inline Shaders::Shared getShaders() const noexcept { return _shaders; };

private:
    bool _is_active{ true };
public:
    /** Enables or disables the renderer.*/
    inline void setActivity(bool state) noexcept { _is_active = state; };
    /** Whether the renderer is enabled or disabled.*/
    inline bool isActive() const noexcept { return _is_active; };
};

template<class Derived>
class Renderer : public RendererBase, public _internal::SharedWindowObject<Derived> {
protected:
    /** Constructor, ensures the renderer is bound to a Window instance.*/
    Renderer(std::shared_ptr<Window> window)
        : _internal::SharedWindowObject<Derived>(window) {};
public:
    /** Virtual destructor, default.*/
    virtual ~Renderer()                     = default;  // Destructor
    /** \cond INTERNAL*/
    Renderer()                              = delete;   // Constructor (default)
    Renderer(const Renderer&)               = delete;   // Copy constructor
    Renderer(Renderer&&)                    = delete;   // Move constructor
    Renderer& operator=(const Renderer&)    = delete;   // Copy assignment
    Renderer& operator=(Renderer&&)         = delete;   // Move assignment
    /** \endcond*/

    using _internal::SharedWindowObject<Derived>::Shared;
};
SSS_GL_END;

#endif // SSS_GL_RENDERER_HPP