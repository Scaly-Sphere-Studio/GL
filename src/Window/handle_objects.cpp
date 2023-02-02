#include "SSS/GL/Window.hpp"

SSS_GL_BEGIN;


Shaders::Shared Window::getPresetShaders(uint32_t id) const noexcept
{
    if (_preset_shaders.count(id) == 0) {
        return nullptr;
    }
    return _preset_shaders.at(id);
}

void Window::addRenderer(RendererBase::Shared renderer, size_t index)
{
    if (index > _renderers.size()) {
        LOG_METHOD_CTX_WRN("Index out of bound", index);
        return;
    }
    if (!renderer) {
        LOG_METHOD_WRN("Given renderer is nullptr");
        return;
    }
    _renderers.insert(_renderers.cbegin() + index, renderer);
}

void Window::addRenderer(RendererBase::Shared renderer)
{
    addRenderer(renderer, _renderers.size());
}

void Window::removeRenderer(RendererBase::Shared renderer)
{
    auto const it = std::find(_renderers.cbegin(), _renderers.cend(), renderer);
    if (it != _renderers.cend())
        _renderers.erase(it);
}

SSS_GL_END;