#include "SSS/GL/Window.hpp"
#include "SSS/GL/Objects/Shaders.hpp"
#include "SSS/GL/Objects/Renderer.hpp"
#include "SSS/GL/Objects/Texture.hpp"

SSS_GL_BEGIN;

Shaders& Window::createShaders(uint32_t id) try
{
    if (id >= static_cast<uint32_t>(Shaders::Preset::First)) {
        throw_exc(CONTEXT_MSG("Given ID is in reserved values", id));
    }
    auto& ptr = _shaders[id];
    ptr.reset(new Shaders(shared_from_this(), id));
    return *ptr;
}
CATCH_AND_RETHROW_METHOD_EXC;

Shaders& Window::createShaders() try
{
    return createShaders(getAvailableID(_shaders));
}
CATCH_AND_RETHROW_METHOD_EXC;

Shaders* Window::getShaders(uint32_t id) const noexcept
{
    if (_shaders.count(id) == 0) {
        return nullptr;
    }
    return _shaders.at(id).get();
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


void Window::removeShaders(uint32_t id)
{
    if (id >= static_cast<uint32_t>(Shaders::Preset::First)) {
        LOG_METHOD_CTX_WRN("Given ID is in reserved values", id);
        return;
    }
    if (_shaders.count(id) != 0) {
        _shaders.erase(_shaders.find(id));
    }
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