#include "SSS/GL/Renderer.hpp"

__SSS_GL_BEGIN

Renderer::Renderer(std::weak_ptr<Window> window)
    : _internal::WindowObject(window)
{
    _shaders.reset(new Shaders(_window));
    _vao.reset(new VAO(_window));
}

void RenderRoutine::render() const
{
    for (Renderer const& renderer : *this) {
        renderer.render();
    }
}

__SSS_GL_END