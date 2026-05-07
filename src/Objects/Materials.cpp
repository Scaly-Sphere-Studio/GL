#include "GL/Objects/Materials.hpp"



SSS_GL_BEGIN;

void RenderState::update()
{
    // Apply RenderState
    //if (blend)
    //{
    //    glEnable(GL_BLEND);
    //    glBlendFunc(blendSrc, blendDst);
    //}
    //else glDisable(GL_BLEND);

    //if (depthclear) glClear(GL_DEPTH_BUFFER_BIT);
    //if (depthTest)
    //{
    //    glDepthMask(depthWrite);
    //    glDepthFunc(depthFunc);
    //}
    //else glDisable(GL_DEPTH_TEST);

    //glCullFace(cullFace);
}

Material::Material(Shaders::Shared shader):_shader(shader) {}

void Material::set(const std::string& name, UniformValue val)
{
   if (_uniforms.contains(name)) 
   {
	   _uniforms.at(name) = val;
   }
   else 
   {
    _uniforms.emplace(name, val);
   }

   _shader->setUniform(name, val);

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
    for (auto& [name, val] : _uniforms) {
        _shader->setUniform(name, val);
    }

	// Push textures
    uint32_t slot = 0;
    for (auto& [name, tex] : _texSlots) {
        glActiveTexture(GL_TEXTURE0 + slot);
        tex->bind();
        _shader->setUniform(name, static_cast<int>(slot));
        slot++;
	}
}

void Material::unbind() const
{
    glActiveTexture(GL_TEXTURE0);
    glUseProgram(0);
}

void Material::setTexture(const std::string &name, const std::filesystem::path path)
{
    _texSlots[name] = SSS::GL::Texture::create(path);
}

void Material::setTexture(const std::string& name, Texture::Shared tex)
{
    _texSlots[name] = tex;
}

void Material::watch() const
{
	_shader->watch();
}

SSS_GL_END;

