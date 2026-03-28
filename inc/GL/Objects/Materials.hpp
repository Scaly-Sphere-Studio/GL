#ifndef SSS_GL_MATERIALS_HPP
#define SSS_GL_MATERIALS_HPP

#include "GL/Objects/Shaders.hpp"

/** @file
 *  Defines class SSS::GL::Materials.
 */

//namespace SSS::Log::GL {
//    /** Logging properties for SSS::GL::Shaders.*/
//    struct Shaders : public LogBase<Materials> {
//        using LOG_STRUCT_BASICS(Log, Materials);
//        bool life_state = false;
//        bool loading = false;
//    };
//}

SSS_GL_BEGIN;

// Ignore warning about STL exports as they're private members
#pragma warning(push, 2)
#pragma warning(disable: 4251)
#pragma warning(disable: 4275)



#pragma warning(pop)

SSS_GL_END;

#endif // SSS_GL_SHADERS_HPP