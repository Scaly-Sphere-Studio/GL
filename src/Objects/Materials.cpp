#include "GL/Objects/Materials.hpp"



SSS_GL_BEGIN;

void RenderState::update()
{
    // Apply RenderState
    if (blend)
    {
        glEnable(GL_BLEND);
        glBlendFunc(blendSrc, blendDst);
    }
    else glDisable(GL_BLEND);

    if (depthclear) glClear(GL_DEPTH_BUFFER_BIT);
    if (depthTest)
    {
        glDepthMask(depthWrite);
        glDepthFunc(depthFunc);
    }
    else glDisable(GL_DEPTH_TEST);

    //glCullFace(cullFace);
}



Material::Material(Shaders::Shared shader):_shader(shader) {}

void Material::set(const std::string& name, UniformValue val)
{
    _uniforms[name] = val;
}

void Material::bind()
{

    if (!_shader) {
        LOG_METHOD_WRN("No shaders bound");
        return;
    }

    _shader->use();
    state.update();


    // Push uniforms
    int texSlot = 0;
    for (auto& [name, val] : _uniforms) {
        _shader->setUniform(name, val);
    }
}

void Material::unbind() const
{
    glActiveTexture(GL_TEXTURE0);
    glUseProgram(0);
}

void Material::watch() const
{
	_shader->watch();
}

SSS_GL_END;

