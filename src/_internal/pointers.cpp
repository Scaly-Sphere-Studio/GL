#include "SSS/GL/_internal/pointers.hpp"
#include "SSS/GL/Window.hpp"

__SSS_GL_BEGIN
__INTERNAL_BEGIN

WindowObject::WindowObject(std::shared_ptr<Window> window)
    : _window(window)
{
    window->use();
}

void WindowObject::throwIfExpired() const
{
    if (hasExpired()) {
        throw_exc("Bound window was destroyed, this instance has expired.");
    }
}

// --- Texture ---

Texture::Texture(std::shared_ptr<Window> window, GLenum given_target) try
    : _internal::AbstractObject(
        window,
        []()->GLuint {
            GLuint id;
            glGenTextures(1, &id);
            return id;
        }()
    ), target(given_target)
{
}
__CATCH_AND_RETHROW_METHOD_EXC

    Texture::~Texture()
{
    glDeleteTextures(1, &id);
}

void Texture::bind() const
{
    throwIfExpired();
    _window.lock()->use();
    glBindTexture(target, id);
}

void Texture::parameteri(GLenum pname, GLint param)
{
    bind();
    glTexParameteri(target, pname, param);
}

void Texture::edit(const GLvoid* pixels, GLsizei width, GLsizei height,
    GLenum format, GLint internalformat, GLenum type, GLint level)
{
    if (pixels == nullptr) {
        return;
    }
    bind();
    glTexImage2D(target, level, internalformat, width, height,
        0, format, type, pixels);
}

__INTERNAL_END

    // --- VAO ---

VAO::VAO(std::shared_ptr<Window> window) try
    : _internal::AbstractObject(
        window,
        []()->GLuint {
            GLuint id;
            glGenVertexArrays(1, &id);
            return id;
        }()
    )
{
}
__CATCH_AND_RETHROW_METHOD_EXC

VAO::~VAO()
{
    glDeleteVertexArrays(1, &id);
}

void VAO::bind() const
{
    throwIfExpired();
    _window.lock()->use();
    glBindVertexArray(id);
}

    // --- VBO ---

VBO::VBO(std::shared_ptr<Window> window) try
    : _internal::AbstractObject(
        window,
        []()->GLuint {
            GLuint id;
            glGenBuffers(1, &id);
            return id;
        }()
    )
{
}
__CATCH_AND_RETHROW_METHOD_EXC

VBO::~VBO()
{
    glDeleteBuffers(1, &id);
}

void VBO::bind() const
{
    throwIfExpired();
    _window.lock()->use();
    glBindBuffer(GL_ARRAY_BUFFER, id);
}

void VBO::unbind() const
{
    throwIfExpired();
    _window.lock()->use();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO::edit(GLsizeiptr size, const void* data, GLenum usage)
{
    bind();
    glBufferData(GL_ARRAY_BUFFER, size, data, usage);
}

    // --- IBO ---

IBO::IBO(std::shared_ptr<Window> window) try
    : _internal::AbstractObject(
        window,
        []()->GLuint {
            GLuint id;
            glGenBuffers(1, &id);
            return id;
        }()
    )
{
}
__CATCH_AND_RETHROW_METHOD_EXC

IBO::~IBO()
{
    glDeleteBuffers(1, &id);
}

void IBO::bind() const
{
    throwIfExpired();
    _window.lock()->use();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
}

void IBO::unbind() const
{
    throwIfExpired();
    _window.lock()->use();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void IBO::edit(GLsizeiptr size, const void* data, GLenum usage)
{
    bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
}

__SSS_GL_END