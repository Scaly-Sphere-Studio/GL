#include "SSS/GL/Window.hpp"
#include "SSS/GL/Objects/Shaders.hpp"
#include "SSS/GL/Objects/Renderer.hpp"
#include "SSS/GL/Objects/Texture.hpp"

SSS_GL_BEGIN;


Shaders::Shared Window::getPresetShaders(uint32_t id) const noexcept
{
    if (_preset_shaders.count(id) == 0) {
        return nullptr;
    }
    return _preset_shaders.at(id);
}

Renderer* Window::getRenderer(uint32_t id) const noexcept
{
    if (_renderers.count(id) == 0) {
        return nullptr;
    }
    return _renderers.at(id).get();
}


Texture& Window::createTexture(uint32_t id) try
{
    auto& ptr = _textures[id];
    ptr.reset(new Texture(shared_from_this(), id));
    return *ptr;
}
CATCH_AND_RETHROW_METHOD_EXC;

Texture& Window::createTexture()
{
    return createTexture(getAvailableID(_textures));
}

Texture* Window::getTexture(uint32_t id) const noexcept
{
    if (_textures.count(id) == 0) {
        return nullptr;
    }
    return _textures.at(id).get();
}


void Window::removeRenderer(uint32_t id)
{
    if (_renderers.count(id) != 0) {
        _renderers.erase(_renderers.find(id));
    }
}

void Window::removeTexture(uint32_t id)
{
    if (_textures.count(id) != 0) {
        _textures.erase(_textures.find(id));
    }
}

SSS_GL_END;