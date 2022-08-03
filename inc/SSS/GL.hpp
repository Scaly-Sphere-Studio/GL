#pragma once

#include "GL/Objects/Texture.hpp"
#include "GL/Objects/Models/PlaneRenderer.hpp"
#include "GL/Objects/Models/LineRenderer.hpp"

/** @file
 *  Header of the
 *  [SSS/GL](https://github.com/Scaly-Sphere-Studio/GL)
 *  library, includes all \c SSS/GL/ headers.
 */

/** @dir SSS/GL
 *  Holds all \b %SSS/GL headers.
 */

/** @dir SSS/GL/internal
 *  Holds \b internal headers for the %SSS/GL library.
 */

/** @dir SSS/GL/Objects
 *  Holds definitions of window \b objects.
 */

/** @dir SSS/GL/Objects/Models
 *  Holds definitions of classes \b derived from SSS::GL::Model
 *  and SSS::GL::Renderer.
 */

/** @namespace SSS::GL
 *  \b OpenGL abstraction using glfw, glm, and glad.
 */

SSS_GL_BEGIN;

template<class Derived>
inline Renderer::Ptr const& Window::createRenderer(uint32_t id)
{
    Renderer::Ptr& ptr = _objects.renderers[id];
    ptr.reset(new Derived(weak_from_this(), id));
    return ptr;
}
 
template<class Derived>
inline Renderer::Ptr const& Window::createRenderer()
{
    try {
        return createRenderer<Derived>(getAvailableID(_objects.renderers));
    }
    catch (std::exception const& e) {
        static Renderer::Ptr n(nullptr);
        LOG_FUNC_ERR(e.what());
        return n;
    }
}
 
template<class T>
Renderer::Ptr const& Renderer::create(std::shared_ptr<Window> win)
{
    if (!win) {
        win = Window::getFirst();
    }
    return win->createRenderer<T>();
}

SSS_GL_END;