#ifndef SSS_GL_MATERIALS_HPP
#define SSS_GL_MATERIALS_HPP

#include "Shaders.hpp"
#include "Texture.hpp"

/** @file
 *  Defines class SSS::GL::Materials.
 */

namespace SSS::Log::GL {
    /** Logging properties for SSS::GL::Shaders.*/
    struct Materials : public LogBase<Materials> {
        using LOG_STRUCT_BASICS(Log, Materials);
        bool life_state = false;
        bool loading = false;
    };
}

SSS_GL_BEGIN;

// Ignore warning about STL exports as they're private members
#pragma warning(push, 2)
#pragma warning(disable: 4251)
#pragma warning(disable: 4275)

struct RenderState 
{
    // Colors
    bool blend          = true;
    GLenum blendSrc     = GL_SRC_ALPHA, blendDst = GL_ONE_MINUS_SRC_ALPHA;
    GLenum cullFace     = GL_BACK;
    
    // Depths
    bool depthTest      = true;
    bool depthWrite     = true;
    bool depthclear     = false;
    GLenum depthFunc    = GL_LEQUAL;

public:
    void update();
};


class SSS_GL_API  Material {
public:
    using UniformValue = Shaders::UniformValue;
    explicit Material(Shaders::Shared shader);
    void set(const std::string& name, UniformValue val);
    void bind();
    void unbind() const;

    //Debugging and hot reloading
    void watch() const;
    RenderState state;
private:
    Shaders::Shared _shader;
    std::unordered_map<std::string, UniformValue>   _uniforms;
    //std::unordered_map<std::string, Texture>        _texSlots;
};


#pragma warning(pop)

SSS_GL_END;

#endif // SSS_GL_SHADERS_HPP