#include "SSS/GL/Window.hpp"

SSS_GL_BEGIN;

void Window::cleanObjects() noexcept
{
    Context const context(_window.get());
    _objects.shaders.erase(
        _objects.shaders.cbegin(),
        _objects.shaders.find(static_cast<uint32_t>(Shaders::Preset::First))
    );
    _objects.renderers.clear();
    _objects.planes.clear();
    _objects.textures.clear();
    _objects.cameras.clear();
}

void Window::createShaders(uint32_t id) try
{
    if (id >= static_cast<uint32_t>(Shaders::Preset::First)) {
        LOG_METHOD_CTX_WRN("Given ID is in reserved values", id);
        return;
    }
    _objects.shaders.try_emplace(id);
    _objects.shaders.at(id).reset(new Shaders(weak_from_this(), id));
}
CATCH_AND_RETHROW_METHOD_EXC;

void Window::removeShaders(uint32_t id)
{
    if (id >= static_cast<uint32_t>(Shaders::Preset::First)) {
        LOG_METHOD_CTX_WRN("Given ID is in reserved values", id);
        return;
    }
    if (_objects.shaders.count(id) != 0) {
        _objects.shaders.erase(_objects.shaders.find(id));
    }
}

void Window::removeRenderer(uint32_t id)
{
    if (_objects.renderers.count(id) != 0) {
        _objects.renderers.erase(_objects.renderers.find(id));
    }
}

void Window::createTexture(uint32_t id) try
{
    _objects.textures.try_emplace(id);
    _objects.textures.at(id).reset(new Texture(weak_from_this(), id));
}
CATCH_AND_RETHROW_METHOD_EXC;

void Window::removeTexture(uint32_t id)
{
    if (_objects.textures.count(id) != 0) {
        _objects.textures.erase(_objects.textures.find(id));
    }
}

void Window::createCamera(uint32_t id) try
{
    _objects.cameras.try_emplace(id);
    _objects.cameras.at(id).reset(new Camera(weak_from_this(), id));
}
CATCH_AND_RETHROW_FUNC_EXC;

void Window::removeCamera(uint32_t id)
{
    if (_objects.cameras.count(id) != 0) {
        _objects.cameras.erase(_objects.cameras.find(id));
    }
}

void Window::createPlane(uint32_t id) try
{
    _objects.planes.try_emplace(id);
    _objects.planes.at(id).reset(new Plane(weak_from_this(), id));
}
CATCH_AND_RETHROW_METHOD_EXC;

void Window::removePlane(uint32_t id)
{
    if (_objects.planes.count(id) != 0) {
        _objects.planes.erase(_objects.planes.find(id));
    }
}

SSS_GL_END;