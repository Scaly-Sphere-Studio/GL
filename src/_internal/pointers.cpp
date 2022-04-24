#include "SSS/GL/_internal/pointers.hpp"
#include "SSS/GL/Window.hpp"

__SSS_GL_BEGIN;
__INTERNAL_BEGIN;

WindowObject::WindowObject(std::weak_ptr<Window> window)
    : _window(window)
{
}

Texture::Texture(std::weak_ptr<Window> window, GLenum given_target) try
    :   _internal::WindowObject(window),
        target(given_target),
        id([&]()->GLuint {
            Context const context(window);
            GLuint id;
            glGenTextures(1, &id);
            return id;
        }())
{
}
__CATCH_AND_RETHROW_METHOD_EXC

    Texture::~Texture()
{
    Context const context(_window);
    glDeleteTextures(1, &id);
}

void Texture::bind() const
{
    Context const context(_window);
    glBindTexture(target, id);
}

void Texture::parameteri(GLenum pname, GLint param)
{
    Context const context(_window);
    bind();
    glTexParameteri(target, pname, param);
}

void Texture::edit(const GLvoid* pixels, GLsizei width, GLsizei height,
    GLenum format, GLint internalformat, GLenum type, GLint level)
{
    if (pixels == nullptr) {
        return;
    }
    Context const context(_window);
    bind();
    glTexImage2D(target, level, internalformat, width, height,
        0, format, type, pixels);
}

__INTERNAL_END;

VAO::VAO(std::weak_ptr<Window> window) try
    :   _internal::WindowObject(window),
        id([&]()->GLuint {
            Context const context(window);
            GLuint id;
            glGenVertexArrays(1, &id);
            return id;
        }())
{
}
__CATCH_AND_RETHROW_METHOD_EXC

VAO::~VAO()
{
    Context const context(_window);
    glDeleteVertexArrays(1, &id);
}

void VAO::bind() const
{
    Context const context(_window);
    glBindVertexArray(id);
}

VBO::VBO(std::weak_ptr<Window> window) try
    :   _internal::WindowObject(window),
        id([&]()->GLuint {
            Context const context(window);
            GLuint id;
            glGenBuffers(1, &id);
            return id;
        }())
{
}
__CATCH_AND_RETHROW_METHOD_EXC

VBO::~VBO()
{
    Context const context(_window);
    glDeleteBuffers(1, &id);
}

void VBO::bind() const
{
    Context const context(_window);
    glBindBuffer(GL_ARRAY_BUFFER, id);
}

void VBO::unbind() const
{
    Context const context(_window);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO::edit(GLsizeiptr size, const void* data, GLenum usage)
{
    Context const context(_window);
    bind();
    glBufferData(GL_ARRAY_BUFFER, size, data, usage);
}

IBO::IBO(std::weak_ptr<Window> window) try
    :   _internal::WindowObject(window),
        id([&]()->GLuint {
            Context const context(window);
            GLuint id;
            glGenBuffers(1, &id);
            return id;
        }())
{
}
__CATCH_AND_RETHROW_METHOD_EXC

IBO::~IBO()
{
    Context const context(_window);
    glDeleteBuffers(1, &id);
}

void IBO::bind() const
{
    Context const context(_window);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
}

void IBO::unbind() const
{
    Context const context(_window);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void IBO::edit(GLsizeiptr size, const void* data, GLenum usage)
{
    Context const context(_window);
    bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
}

__SSS_GL_END;