#include "SSS/GL/_pointers.hpp"

__SSS_GL_BEGIN

    // --- VAO ---

VAO::VAO() try
    : _internal::AbstractObject([]()->GLuint {
        GLuint id;
        glGenVertexArrays(1, &id);
        return id;
    }())
{
}
__CATCH_AND_RETHROW_METHOD_EXC

VAO::~VAO()
{
    glDeleteVertexArrays(1, &id);
}

void VAO::bind() const
{
    glBindVertexArray(id);
}

    // --- VBO ---

VBO::VBO() try
    : _internal::AbstractObject([]()->GLuint {
        GLuint id;
        glGenBuffers(1, &id);
        return id;
    }())
{
}
__CATCH_AND_RETHROW_METHOD_EXC

VBO::~VBO()
{
    glDeleteBuffers(1, &id);
}

void VBO::bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, id);
}

void VBO::unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO::edit(GLsizeiptr size, const void* data, GLenum usage)
{
    bind();
    glBufferData(GL_ARRAY_BUFFER, size, data, usage);
}

    // --- IBO ---

IBO::IBO() try
    : _internal::AbstractObject([]()->GLuint {
        GLuint id;
        glGenBuffers(1, &id);
        return id;
    }())
{
}
__CATCH_AND_RETHROW_METHOD_EXC

IBO::~IBO()
{
    glDeleteBuffers(1, &id);
}

void IBO::bind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
}

void IBO::unbind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void IBO::edit(GLsizeiptr size, const void* data, GLenum usage)
{
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
}

    // --- Texture ---

Texture::Texture(GLenum given_target) try
    : _internal::AbstractObject(
        []()->GLuint {
            GLuint id;
            glGenTextures(1, &id);
            return id;
        }()
    ), target(given_target), context(glfwGetCurrentContext())
{
}
__CATCH_AND_RETHROW_METHOD_EXC

Texture::~Texture()
{
    glDeleteTextures(1, &id);
}

void Texture::bind() const
{
    glBindTexture(target, id);
}

void Texture::parameteri(GLenum pname, GLint param)
{
    glTexParameteri(target, pname, param);
}

void Texture::edit(const GLvoid* pixels, GLsizei width, GLsizei height,
    GLenum format, GLint internalformat, GLenum type, GLint level)
{
    glTexImage2D(target, level, internalformat, width, height,
        0, format, type, pixels);
}

__SSS_GL_END