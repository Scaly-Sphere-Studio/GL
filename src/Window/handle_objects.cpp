#include "SSS/GL/Window.hpp"

SSS_GL_BEGIN;


Shaders::Shared Window::getPresetShaders(uint32_t id) const noexcept
{
    if (_preset_shaders.count(id) == 0) {
        return nullptr;
    }
    return _preset_shaders.at(id);
}

void Window::addRenderer(RendererVariant renderer, size_t index)
{
    if (index > _renderers.size()) {
        LOG_METHOD_CTX_WRN("Index out of bound", index);
        return;
    }
    if (std::visit([](auto&& var) { return !var; }, renderer)) {
        LOG_METHOD_WRN("Given renderer is nullptr");

    }
    _renderers.insert(_renderers.cbegin() + index, renderer);
}

void Window::addRenderer(RendererVariant renderer)
{
    addRenderer(renderer, _renderers.size());
}

void Window::removeRenderer(RendererVariant renderer)
{
    auto const it = std::find(_renderers.cbegin(), _renderers.cend(), renderer);
    if (it != _renderers.cend())
        _renderers.erase(it);
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


void Window::removeTexture(uint32_t id)
{
    if (_textures.count(id) != 0) {
        _textures.erase(_textures.find(id));
    }
}

SSS_GL_END;