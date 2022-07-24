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
    _objects.textures.clear();
}

Shaders::Ptr const& Window::createShaders(uint32_t id) try
{
    if (id >= static_cast<uint32_t>(Shaders::Preset::First)) {
        static Shaders::Ptr n(nullptr);
        LOG_METHOD_CTX_WRN("Given ID is in reserved values", id);
        return n;
    }
    Shaders::Ptr& ptr = _objects.shaders[id];
    ptr.reset(new Shaders(weak_from_this(), id));
    return ptr;
}
CATCH_AND_RETHROW_METHOD_EXC;

Shaders::Ptr const& Window::createShaders()
{
    try {
        return createShaders(getAvailableID(_objects.shaders));
    }
    catch (std::exception const& e) {
        static Shaders::Ptr n(nullptr);
        LOG_FUNC_ERR(e.what());
        return n;
    }
}

Texture::Ptr const& Window::createTexture(uint32_t id) try
{
    Texture::Ptr& ptr = _objects.textures[id];
    ptr.reset(new Texture(weak_from_this(), id));
    return ptr;
}
CATCH_AND_RETHROW_METHOD_EXC;

Texture::Ptr const& Window::createTexture()
{
    try {
        return createTexture(getAvailableID(_objects.textures));
    }
    catch (std::exception const& e) {
        static Texture::Ptr n(nullptr);
        LOG_FUNC_ERR(e.what());
        return n;
    }
}

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

void Window::removeTexture(uint32_t id)
{
    if (_objects.textures.count(id) != 0) {
        _objects.textures.erase(_objects.textures.find(id));
    }
}

SSS_GL_END;