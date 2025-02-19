#ifndef SSS_GL_RENDERER_HPP
#define SSS_GL_RENDERER_HPP

#include "Shaders.hpp"

/** @file
 *  Defines abstract class SSS::GL::Renderer.
 */

SSS_GL_BEGIN;

// Ignore warning about STL exports as they're private members
#pragma warning(push, 2)
#pragma warning(disable: 4251)
#pragma warning(disable: 4275)

/** Abstract class for specialized rendering logic.
 *  Don't inherit from this, only used to be stored in Window instances
 *  @sa Window::createRenderer(), Plane::Renderer
 */
class SSS_GL_API RendererBase {
public:
    virtual std::shared_ptr<RendererBase> getSharedBase() noexcept = 0;
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

#pragma warning(pop)

template<class Derived>
class Renderer : public RendererBase, public SharedClass<Derived> {
protected:
    Renderer()                              = default;  // Constructor
public:
    /** Virtual destructor, default.*/
    virtual ~Renderer()                     = default;  // Destructor
    /** \cond INTERNAL*/
    Renderer(const Renderer&)               = delete;   // Copy constructor
    Renderer(Renderer&&)                    = delete;   // Move constructor
    Renderer& operator=(const Renderer&)    = delete;   // Copy assignment
    Renderer& operator=(Renderer&&)         = delete;   // Move assignment
    /** \endcond*/

    virtual std::shared_ptr<RendererBase> getSharedBase() noexcept final {
        return SharedClass<Derived>::shared_from_this();
    };
};

SSS_GL_END;

#endif // SSS_GL_RENDERER_HPP