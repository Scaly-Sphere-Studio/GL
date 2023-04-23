#ifndef SSS_GL_LUA_HPP
#define SSS_GL_LUA_HPP

#define SOL_ALL_SAFETIES_ON 1
#define SOL_STRINGS_ARE_NUMBERS 1
#include <sol/sol.hpp>
#include "Objects/Basic.hpp"

SSS_GL_BEGIN;

SSS_GL_API void lua_setup_GL(sol::state& lua);

SSS_GL_END;

#endif // SSS_GL_LUA_HPP