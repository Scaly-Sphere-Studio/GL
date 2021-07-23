#include "SSS/GL/_internal/pointers.hpp"
#include "SSS/GL/Context.hpp"

__SSS_GL_BEGIN
__INTERNAL_BEGIN

ContextObject::ContextObject(GLFWwindow const* context)
    : _context(context)
{
}

// --- Texture ---

Texture::Texture(GLFWwindow const* context, GLenum given_target) try
    : _internal::AbstractObject(
        context,
        [&]()->GLuint {
            ContextLocker const context_manager(_context);
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
    ContextLocker const context_manager(_context);
    glDeleteTextures(1, &id);
}

void Texture::bind() const
{
    ContextLocker const context_manager(_context);
    glBindTexture(target, id);
}

void Texture::parameteri(GLenum pname, GLint param)
{
    ContextLocker const context_manager(_context);
    bind();
    glTexParameteri(target, pname, param);
}

void Texture::edit(const GLvoid* pixels, GLsizei width, GLsizei height,
    GLenum format, GLint internalformat, GLenum type, GLint level)
{
    if (pixels == nullptr) {
        return;
    }
    ContextLocker const context_manager(_context);
    bind();
    glTexImage2D(target, level, internalformat, width, height,
        0, format, type, pixels);
}

__INTERNAL_END

    // --- VAO ---

VAO::VAO(GLFWwindow const* context) try
    : _internal::AbstractObject(
        context,
        [&]()->GLuint {
            ContextLocker const context_manager(_context);
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
    ContextLocker const context_manager(_context);
    glDeleteVertexArrays(1, &id);
}

void VAO::bind() const
{
    ContextLocker const context_manager(_context);
    glBindVertexArray(id);
}

    // --- VBO ---

VBO::VBO(GLFWwindow const* context) try
    : _internal::AbstractObject(
        context,
        [&]()->GLuint {
            ContextLocker const context_manager(_context);
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
    ContextLocker const context_manager(_context);
    glDeleteBuffers(1, &id);
}

void VBO::bind() const
{
    ContextLocker const context_manager(_context);
    glBindBuffer(GL_ARRAY_BUFFER, id);
}

void VBO::unbind() const
{
    ContextLocker const context_manager(_context);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO::edit(GLsizeiptr size, const void* data, GLenum usage)
{
    ContextLocker const context_manager(_context);
    bind();
    glBufferData(GL_ARRAY_BUFFER, size, data, usage);
    unbind();
}

    // --- IBO ---

IBO::IBO(GLFWwindow const* context) try
    : _internal::AbstractObject(
        context,
        [&]()->GLuint {
            ContextLocker const context_manager(_context);
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
    ContextLocker const context_manager(_context);
    glDeleteBuffers(1, &id);
}

void IBO::bind() const
{
    ContextLocker const context_manager(_context);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
}

void IBO::unbind() const
{
    ContextLocker const context_manager(_context);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void IBO::edit(GLsizeiptr size, const void* data, GLenum usage)
{
    ContextLocker const context_manager(_context);
    bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
    unbind();
}

__SSS_GL_END