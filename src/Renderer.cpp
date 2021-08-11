#include "SSS/GL/Renderer.hpp"

__SSS_GL_BEGIN

Renderer::Renderer(std::weak_ptr<Window> window)
    : _internal::WindowObject(window)
{
    _shaders.reset(new Shaders(_window));
    _vao.reset(new VAO(_window));
    _vbo.reset(new VBO(_window));
    _ibo.reset(new IBO(_window));
}

__SSS_GL_END