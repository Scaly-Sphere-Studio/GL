#include "SSS/GL/_internal/pointers.hpp"
#include "SSS/GL/Context.hpp"

__SSS_GL_BEGIN
__INTERNAL_BEGIN

ContextObject::ContextObject(std::shared_ptr<Context> context)
    : _context(context)
{
}

// --- Texture ---

Texture::Texture(std::shared_ptr<Context> context, GLenum given_target) try
    : _internal::AbstractObject(
        context,
        [&]()->GLuint {
            ContextManager const context_manager(context);
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
    ContextManager const context_manager(_context.lock());
    glDeleteTextures(1, &id);
}

void Texture::bind() const
{
    ContextManager const context_manager(_context.lock());
    glBindTexture(target, id);
}

void Texture::parameteri(GLenum pname, GLint param)
{
    ContextManager const context_manager(_context.lock());
    bind();
    glTexParameteri(target, pname, param);
}

void Texture::edit(const GLvoid* pixels, GLsizei width, GLsizei height,
    GLenum format, GLint internalformat, GLenum type, GLint level)
{
    if (pixels == nullptr) {
        return;
    }
    ContextManager const context_manager(_context.lock());
    bind();
    glTexImage2D(target, level, internalformat, width, height,
        0, format, type, pixels);
}

__INTERNAL_END

    // --- VAO ---

VAO::VAO(std::shared_ptr<Context> context) try
    : _internal::AbstractObject(
        context,
        [&]()->GLuint {
            ContextManager const context_manager(context);
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
    ContextManager const context_manager(_context.lock());
    glDeleteVertexArrays(1, &id);
}

void VAO::bind() const
{
    ContextManager const context_manager(_context.lock());
    glBindVertexArray(id);
}

    // --- VBO ---

VBO::VBO(std::shared_ptr<Context> context) try
    : _internal::AbstractObject(
        context,
        [&]()->GLuint {
            ContextManager const context_manager(context);
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
    ContextManager const context_manager(_context.lock());
    glDeleteBuffers(1, &id);
}

void VBO::bind() const
{
    ContextManager const context_manager(_context.lock());
    glBindBuffer(GL_ARRAY_BUFFER, id);
}

void VBO::unbind() const
{
    ContextManager const context_manager(_context.lock());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO::edit(GLsizeiptr size, const void* data, GLenum usage)
{
    ContextManager const context_manager(_context.lock());
    bind();
    glBufferData(GL_ARRAY_BUFFER, size, data, usage);
    unbind();
}

    // --- IBO ---

IBO::IBO(std::shared_ptr<Context> context) try
    : _internal::AbstractObject(
        context,
        [&]()->GLuint {
            ContextManager const context_manager(context);
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
    ContextManager const context_manager(_context.lock());
    glDeleteBuffers(1, &id);
}

void IBO::bind() const
{
    ContextManager const context_manager(_context.lock());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
}

void IBO::unbind() const
{
    ContextManager const context_manager(_context.lock());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void IBO::edit(GLsizeiptr size, const void* data, GLenum usage)
{
    ContextManager const context_manager(_context.lock());
    bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
    unbind();
}

__SSS_GL_END