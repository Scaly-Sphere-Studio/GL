#pragma once

#include "_includes.hpp"

__SSS_GL_BEGIN;

class Window; // Pre-declaration of Window class.

__INTERNAL_BEGIN;

// This class is to be inherited in all classes whose instances are bounded
// to a specific Window instance. 
class WindowObject {
public:
    // Delete default constructor to enforce bounding to a Window instance
    WindowObject() = delete;
protected:
    WindowObject(std::weak_ptr<Window> window) : _window(window) {};
    std::weak_ptr<Window> _window;
};

__INTERNAL_END;

namespace Basic {
    struct Texture : public _internal::WindowObject {
        using Ptr = std::unique_ptr<Texture>;
        Texture(std::weak_ptr<Window> window, GLenum given_target);
        ~Texture();
        void bind() const;
        void parameteri(GLenum pname, GLint param);
        void edit(const GLvoid* pixels, GLsizei width, GLsizei height,
            GLenum format = GL_RGBA, GLint internalformat = GL_RGBA,
            GLenum type = GL_UNSIGNED_BYTE, GLint level = 0);
        GLenum const target;
        GLuint const id;
    };

    struct VAO : public _internal::WindowObject {
        using Ptr = std::unique_ptr<VAO>;
        VAO(std::weak_ptr<Window> window);
        ~VAO();
        void bind() const;
        GLuint const id;
    };

    struct VBO : public _internal::WindowObject {
        using Ptr = std::unique_ptr<VBO>;
        VBO(std::weak_ptr<Window> window);
        ~VBO();
        void bind() const;
        void unbind() const;
        void edit(GLsizeiptr size, const void* data, GLenum usage);
        GLuint const id;
    };

    struct IBO : public _internal::WindowObject {
        using Ptr = std::unique_ptr<IBO>;
        IBO(std::weak_ptr<Window> window);
        ~IBO();
        void bind() const;
        void unbind() const;
        void edit(GLsizeiptr size, const void* data, GLenum usage);
        GLuint const id;
    };
}

__SSS_GL_END;