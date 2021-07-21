#pragma once

#include <SSS/Commons/pointers.hpp>
#include "includes.hpp"

__SSS_GL_BEGIN

class Context; // Pre-declaration of Context class.

__INTERNAL_BEGIN

using GLFWwindow_Ptr = C_Ptr
    <GLFWwindow, void(*)(GLFWwindow*), glfwDestroyWindow>;

// This class is to be inherited in all classes whose instances are bounded
// to a specific Window instance. 
class ContextObject {
public:
    // Delete default constructor to enforce bounding to a Window instance
    ContextObject() = delete;
protected:
    ContextObject(std::shared_ptr<Context> context);
    std::weak_ptr<Context> _context;
};

struct AbstractObject : public ContextObject {
    inline AbstractObject(std::shared_ptr<Context> context, GLuint given_id)
        : ContextObject(context), id(given_id) {};
    virtual void bind() const = 0;
    GLuint const id;
};

struct Texture : public AbstractObject {
    using Ptr = std::unique_ptr<Texture>;
    using Shared = std::shared_ptr<Texture>;
    Texture(std::shared_ptr<Context> context, GLenum given_target);
    ~Texture();
    virtual void bind() const;
    void parameteri(GLenum pname, GLint param);
    void edit(const GLvoid* pixels, GLsizei width, GLsizei height,
        GLenum format = GL_RGBA, GLint internalformat = GL_RGBA,
        GLenum type = GL_UNSIGNED_BYTE, GLint level = 0);
    GLenum const target;
};

__INTERNAL_END

struct VAO : public _internal::AbstractObject {
    using Ptr = std::unique_ptr<VAO>;
    using Shared = std::shared_ptr<VAO>;
    VAO(std::shared_ptr<Context> context);
    ~VAO();
    virtual void bind() const;
};

struct VBO : public _internal::AbstractObject {
    using Ptr = std::unique_ptr<VBO>;
    using Shared = std::shared_ptr<VBO>;
    VBO(std::shared_ptr<Context> context);
    ~VBO();
    virtual void bind() const;
    virtual void unbind() const;
    void edit(GLsizeiptr size, const void* data, GLenum usage);
};

struct IBO : public _internal::AbstractObject {
    using Ptr = std::unique_ptr<IBO>;
    using Shared = std::shared_ptr<IBO>;
    IBO(std::shared_ptr<Context> context);
    ~IBO();
    virtual void bind() const;
    virtual void unbind() const;
    void edit(GLsizeiptr size, const void* data, GLenum usage);
};

__SSS_GL_END