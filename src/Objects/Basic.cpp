#include "SSS/GL/Objects/Basic.hpp"
#include "SSS/GL/Window.hpp"

SSS_GL_BEGIN;

INTERNAL_BEGIN;

WindowObject::WindowObject(std::shared_ptr<Window> window)
{
    _changeWindow(window);
}

void WindowObject::_changeWindow(std::shared_ptr<Window> window)
{
    _window = window;
    _glfw_window = getWindow()->getGLFWwindow();
}

std::shared_ptr<Window> const WindowObject::getWindow() const
{
    Window::Shared const window = _window.lock();
    if (!window) {
        throw_exc("Trying to use window-dependent object without bounding a valid Window.");
    }
    return window;
}

Context const WindowObject::getContext() const
{
    return Context(_glfw_window);
}

INTERNAL_END;

namespace Basic {

    Texture::Texture(std::shared_ptr<Window> window, GLenum given_target) try
        :   _internal::WindowObject(window),
            id([&]()->GLuint {
                Context const context(window);
                GLuint id;
                glGenTextures(1, &id);
                return id;
            }())
    {
        setTarget(given_target);
    }
    CATCH_AND_RETHROW_METHOD_EXC;

    Texture::~Texture()
    {
        try {
            Context const context = getContext();
            glDeleteTextures(1, &id);
        }
        catch (...) {
            LOG_CTX_WRN(THIS_NAME, "Could not delete properly: no valid Window bound.");
            return;
        };
    }

    void Texture::bind() const
    {
        Context const context = getContext();
        glBindTexture(_target, id);
    }

    void Texture::setTarget(GLenum new_target) {
        Context const context = getContext();
        _target = new_target;
        bind();
        switch (_target)
        {
        case GL_TEXTURE_2D:
        case GL_TEXTURE_RECTANGLE:
            glTexImage2D(_target, 0, GL_RGBA, _width, _height,
                0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            break;

        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_3D:
            glTexImage3D(_target, 0, GL_RGBA, _width, _height, _depth,
                0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            break;

        default:
            throw_exc(METHOD_MSG("Given target is NOT handled by SSS/GL."));
        }
    }

    void Texture::parameteri(GLenum pname, GLint param)
    {
        Context const context = getContext();
        bind();
        glTexParameteri(_target, pname, param);
    }

    void Texture::editSettings(int width, int height, int depth) try
    {
        if (_width == width && _height == height && _depth == depth) {
            return;
        }

        Context const context = getContext();
        bind();

        _width = width;
        _height = height;
        _depth = depth;

        switch (_target)
        {
        case GL_TEXTURE_2D:
        case GL_TEXTURE_RECTANGLE:
            glTexImage2D(_target, 0, GL_RGBA, _width, _height,
                0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            break;

        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_3D:
            glTexImage3D(_target, 0, GL_RGBA, _width, _height, _depth,
                0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            break;

        default:
            throw_exc(METHOD_MSG("Given target is NOT handled by SSS/GL."));
        }
    }
    CATCH_AND_RETHROW_METHOD_EXC;

    void Texture::editPixels(const GLvoid* pixels, int z_offset) try
    {
        if (pixels == nullptr) {
            return;
        }
        Context const context = getContext();
        bind();

        switch (_target)
        {
        case GL_TEXTURE_2D:
        case GL_TEXTURE_RECTANGLE:
            glTexSubImage2D(_target, 0, 0, 0, _width, _height,
                GL_RGBA, GL_UNSIGNED_BYTE, pixels);
            break;

        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_3D:
            glTexSubImage3D(_target, 0, 0, 0, z_offset, _width, _height, 1,
                GL_RGBA, GL_UNSIGNED_BYTE, pixels);
            break;

        default:
            throw_exc(METHOD_MSG("Given target is NOT handled by SSS/GL."));
        }
    }
    CATCH_AND_RETHROW_METHOD_EXC;


    VAO::VAO(std::shared_ptr<Window> window) try
        :   _internal::WindowObject(window),
            id([&]()->GLuint {
                Context const context(window);
                GLuint id;
                glGenVertexArrays(1, &id);
                return id;
            }())
    {
    }
    CATCH_AND_RETHROW_METHOD_EXC;

    VAO::~VAO()
    {
        try {
            Context const context = getContext();
            glDeleteVertexArrays(1, &id);
        }
        catch (...) {
            LOG_CTX_WRN(THIS_NAME, "Could not delete properly: no valid Window bound.");
            return;
        };
    }

    void VAO::bind() const
    {
        Context const context = getContext();
        glBindVertexArray(id);
    }

    void VAO::unbind() const
    {
        Context const context = getContext();
        glBindVertexArray(0);
    }

    VBO::VBO(std::shared_ptr<Window> window) try
        :   _internal::WindowObject(window),
            id([&]()->GLuint {
                Context const context(window);
                GLuint id;
                glGenBuffers(1, &id);
                return id;
            }())
    {
    }
    CATCH_AND_RETHROW_METHOD_EXC;


    VBO::~VBO()
    {
        try {
            Context const context = getContext();
            glDeleteBuffers(1, &id);
        }
        catch (...) {
            LOG_CTX_WRN(THIS_NAME, "Could not delete properly: no valid Window bound.");
            return;
        };
    }

    void VBO::bind() const
    {
        Context const context = getContext();
        glBindBuffer(GL_ARRAY_BUFFER, id);
    }

    void VBO::edit(GLsizeiptr size, const void* data, GLenum usage)
    {
        Context const context = getContext();
        bind();
        glBufferData(GL_ARRAY_BUFFER, size, data, usage);
    }


    IBO::IBO(std::shared_ptr<Window> window) try
        :   _internal::WindowObject(window),
            id([&]()->GLuint {
                Context const context(window);
                GLuint id;
                glGenBuffers(1, &id);
                return id;
            }())
    {
    }
    CATCH_AND_RETHROW_METHOD_EXC;

    IBO::~IBO()
    {
        try {
            Context const context = getContext();
            glDeleteBuffers(1, &id);
        }
        catch (...) {
            LOG_CTX_WRN(THIS_NAME, "Could not delete properly: no valid Window bound.");
            return;
        };
    }

    void IBO::bind() const
    {
        Context const context = getContext();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
    }

    void IBO::edit(GLsizeiptr size, const void* data, GLenum usage)
    {
        Context const context = getContext();
        bind();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
    }
}
SSS_GL_END;