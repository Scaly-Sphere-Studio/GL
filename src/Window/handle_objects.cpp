#include "GL/Window.hpp"
#include "GL/Objects/Shaders.hpp"
#include "GL/Objects/Models/PlaneRenderer.hpp"

SSS_GL_BEGIN;


Shaders::Shared Window::getPresetShaders(uint32_t id) noexcept
{
    if (_main._preset_shaders.count(id) == 0) {
        return nullptr;
    }
    return _main._preset_shaders.at(id);
}

void Window::addRenderer(std::shared_ptr<RendererBase> renderer, size_t index)
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

void Window::addRenderer(std::shared_ptr<RendererBase> renderer)
{
    addRenderer(renderer, _renderers.size());
}

void Window::removeRenderer(std::shared_ptr<RendererBase> renderer)
{
    auto const it = std::find(_renderers.cbegin(), _renderers.cend(), renderer);
    if (it != _renderers.cend())
        _renderers.erase(it);
}

SSS_GL_END;