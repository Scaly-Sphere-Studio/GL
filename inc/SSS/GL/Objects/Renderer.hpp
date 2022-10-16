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
    Renderer(std::weak_ptr<Window> window, uint32_t id)
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
    
    /** Unique ptr stored in Window objects.*/
    using Ptr = std::unique_ptr<Renderer>;
    /** Creates a Ptr in Window objects at given ID.
     *  @sa Window::removeRenderer()
     */
    template <class T>
    static Ptr const& create(std::shared_ptr<Window> win = nullptr);
    
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
    uint32_t _shaders_id{ 0 };
public:
    /** Sets the Shaders ID to be used in render().*/
    inline void setShadersID(uint32_t id) noexcept { _shaders_id = id; };
    /** Retrieves the set Shaders ID to be used in render().*/
    inline uint32_t getShadersID() const noexcept { return _shaders_id; };

private:
    bool _is_active{ true };
public:
    /** Enables or disables the renderer.*/
    inline void setActivity(bool state) noexcept { _is_active = state; };
    /** Whether the renderer is enabled or disabled.*/
    inline bool isActive() const noexcept { return _is_active; };
};

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