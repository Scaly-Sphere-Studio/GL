#ifndef SSS_GL_LUA_HPP
#define SSS_GL_LUA_HPP

#include <sol/sol.hpp>
#include "Objects/Basic.hpp"

SSS_GL_BEGIN;

SSS_GL_API void lua_setup_GL(sol::state& lua);

SSS_GL_END;

#endif // SSS_GL_LUA_HPP