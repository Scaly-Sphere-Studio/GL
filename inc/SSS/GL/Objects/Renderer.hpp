#ifndef SSS_GL_RENDERER_HPP
#define SSS_GL_RENDERER_HPP

#include "SSS/GL/Objects/Shaders.hpp"

/** @file
 *  Defines abstract class SSS::GL::Renderer.
 */

SSS_GL_BEGIN;

/** Abstract class for specialized rendering logic.
 *  @sa Window::createRenderer(), Plane::Renderer
 */
class Renderer : public _internal::WindowObjectWithID {
protected:
    /** Constructor, ensures the renderer is bound to a Window instance.*/
    Renderer(std::shared_ptr<Window> window, uint32_t id)
        : _internal::WindowObjectWithID(window, id) {};
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
    
    /** Creates an instance stored in Window at given ID.
     *  @sa Window::removeRenderer()
     */
    template<typename Derived = Renderer>
    static Derived& create(std::shared_ptr<Window> win);
    template<typename Derived = Renderer>
    static Derived& create();
    
    /** Casts instance back to its derived allocated class.
     *  If no window is specified, the first one (Window::getFirst()) is used.\n
     *  Throws if specified type differs from actual original type.
     */
    template< class Derived,
        typename = std::enable_if_t< std::is_base_of_v<Renderer, Derived> > >
    Derived& castAs();

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
inline Derived& Window::createRenderer(uint32_t id) try
{
    auto& ptr = _renderers[id];
    ptr.reset(new Derived(shared_from_this(), id));
    return ptr->castAs<Derived>();
}
CATCH_AND_RETHROW_METHOD_EXC;

template<class Derived>
inline Derived& Window::createRenderer() try
{
    return createRenderer<Derived>(getAvailableID(_renderers));
}
CATCH_AND_RETHROW_METHOD_EXC;

template<class Derived>
Derived* Window::getRenderer(uint32_t id) const noexcept
{
    if (_renderers.count(id) == 0) {
        return nullptr;
    }
    return _renderers.at(id)->castAs<Derived>();
}

template<class Derived>
Derived& Renderer::create(std::shared_ptr<Window> win) try
{
    if (!win) {
        throw_exc("Given Window is nullptr.");
    }
    return win->createRenderer<Derived>();
}
CATCH_AND_RETHROW_FUNC_EXC;

template<class Derived>
Derived& Renderer::create() try
{
    std::shared_ptr<Window> win = Window::getFirst();
    if (!win) {
        throw_exc("No Window instance exists.");
    }
    return win->createRenderer<Derived>();
}
CATCH_AND_RETHROW_FUNC_EXC;

template< class Derived, typename >
inline Derived& Renderer::castAs()
{
    Derived* ptr = dynamic_cast<Derived*>(this);
    if (ptr == nullptr) {
        throw_exc(METHOD_MSG("Specified type differs from original"));
    }
    return *ptr;
};

SSS_GL_END;

#endif // SSS_GL_RENDERER_HPP