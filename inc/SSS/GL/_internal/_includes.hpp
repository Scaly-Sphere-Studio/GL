#pragma once

/** @dir SSS/GL/_internal
 *  Holds \b internal headers for the %SSS/Text-Rendering library.
 */

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
#include <SSS/Text-Rendering.hpp>

/** \cond INCLUDE*/
// STL
#include <array>
#include <deque>
#include <fstream>
#include <mutex>
#include <shared_mutex>
#include <algorithm>
/** \endcond*/

/** Declares the SSS::GL namespace.
 *  Further code will be nested in the SSS::GL namespace.\n
 *  Should be used in pair with with #__SSS_GL_END.
 */
#define __SSS_GL_BEGIN __SSS_BEGIN; namespace GL {
/** Closes the SSS::GL namespace declaration.
 *  Further code will no longer be nested in the SSS::GL namespace.\n
 *  Should be used in pair with with #__SSS_GL_BEGIN.
 */
#define __SSS_GL_END __SSS_END; }
