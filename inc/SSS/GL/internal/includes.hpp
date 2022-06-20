#pragma once

/** @file
 *  Base header including resources and defining macros used by other headers.
 */

// OpenGL headers
#include <glad/glad.h>              // glad
#include <GLFW/glfw3.h>             // glfw
#include <glm/glm.hpp>              // glm
#include <glm/ext.hpp>              // glm
#include <glm/gtc/quaternion.hpp>   // glm
#include <glm/gtx/quaternion.hpp>   // glm

// SSS libs
#include <SSS/Commons.hpp>

/** \cond INCLUDE*/
// STL
#include <array>
#include <deque>
#include <list>
#include <map>
#include <fstream>
#include <mutex>
#include <shared_mutex>
#include <algorithm>
/** \endcond*/

/** Declares the SSS::GL namespace.
 *  Further code will be nested in the SSS::GL namespace.\n
 *  Should be used in pair with with #SSS_GL_END.
 */
#define SSS_GL_BEGIN SSS_BEGIN; namespace GL {
/** Closes the SSS::GL namespace declaration.
 *  Further code will no longer be nested in the SSS::GL namespace.\n
 *  Should be used in pair with with #SSS_GL_BEGIN.
 */
#define SSS_GL_END SSS_END; }

/** \cond INTERNAL*/

/** Logs the given message with "SSS/GL: " prepended to it.*/
#define LOG_GL_MSG(X) LOG_CTX_MSG("SSS/GL", X)

/** Logs the given message with "SSS/GL: %window_name%: " prepended to it.*/
#define LOG_WNDW_MSG(win, X) LOG_GL_MSG(win->getTitle(), X)

/** \endcond*/

/** Holds all SSS::GL related log flags.*/
namespace SSS::Log::GL {
    LOG_NAMESPACE_BASICS(Log);
}
