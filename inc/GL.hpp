#ifndef SSS_GL_HPP
#define SSS_GL_HPP

#include "GL/Window.hpp"
#include "GL/Objects/Models/PlaneRenderer.hpp"
#include "GL/Objects/Models/LineRenderer.hpp"
#include "Settings/Theme.h"
#ifdef SSS_LUA
#include "GL/Lua.hpp"
#endif // SSS_LUA

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

#endif // SSS_GL_HPP