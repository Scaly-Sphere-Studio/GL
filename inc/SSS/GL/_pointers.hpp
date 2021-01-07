#pragma once

#include <SSS/Commons/pointers.hpp>
#include "SSS/GL/_includes.hpp"

__SSS_GL_BEGIN
__INTERNAL_BEGIN

using GLFWwindow_Ptr = C_Ptr
    <GLFWwindow, void(*)(GLFWwindow*), glfwDestroyWindow>;

struct AbstractObject {
    inline AbstractObject(GLuint given_id) : id(given_id) {};
    virtual void bind() const = 0;
    GLuint const id;
};

__INTERNAL_END

struct VAO : _internal::AbstractObject {
    using Ptr = std::unique_ptr<VAO>;
    using Shared = std::shared_ptr<VAO>;
    VAO();
    ~VAO();
    virtual void bind() const;
};

struct VBO : _internal::AbstractObject {
    using Ptr = std::unique_ptr<VBO>;
    using Shared = std::shared_ptr<VBO>;
    VBO();
    ~VBO();
    virtual void bind() const;
    void edit(GLsizeiptr size, const void* data, GLenum usage);
};

struct IBO : _internal::AbstractObject {
    using Ptr = std::unique_ptr<IBO>;
    using Shared = std::shared_ptr<IBO>;
    IBO();
    ~IBO();
    virtual void bind() const;
    void edit(GLsizeiptr size, const void* data, GLenum usage);
};

struct Texture : _internal::AbstractObject {
    using Ptr = std::unique_ptr<Texture>;
    using Shared = std::shared_ptr<Texture>;
    Texture(GLenum given_target);
    ~Texture();
    virtual void bind() const;
    void parameteri(GLenum pname, GLint param);
    void edit(GLint level, GLint internalformat, GLsizei width, GLsizei height,
        GLenum format, GLenum type, const GLvoid* pixels);
    GLenum const target;
};

__SSS_GL_END