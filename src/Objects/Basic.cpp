#include "GL/Objects/Basic.hpp"
#include "GL/Globals.hpp"

SSS_GL_BEGIN;

namespace Basic {

    Texture::Texture(GLenum given_target) try
        :   id([&]()->GLuint {
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
        glDeleteTextures(1, &id);
    }

    void Texture::bind() const
    {
        glBindTexture(_target, id);
    }

    void Texture::setTarget(GLenum new_target) {
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
        bind();
        glTexParameteri(_target, pname, param);
    }

    void Texture::editSettings(int width, int height, int depth) try
    {
        if (_width == width && _height == height && _depth == depth) {
            return;
        }

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


    VAO::VAO() try
        :   id([&]()->GLuint {
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
            glDeleteVertexArrays(1, &id);
        }
        catch (...) {
            LOG_CTX_WRN(THIS_NAME, "Could not delete properly: no valid Window bound.");
            return;
        };
    }

    void VAO::bind() const
    {
        glBindVertexArray(id);
    }

    void VAO::unbind() const
    {
        glBindVertexArray(0);
    }

    VBO::VBO() try
        :   id([&]()->GLuint {
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
            glDeleteBuffers(1, &id);
        }
        catch (...) {
            LOG_CTX_WRN(THIS_NAME, "Could not delete properly: no valid Window bound.");
            return;
        };
    }

    void VBO::bind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, id);
    }

    void VBO::edit(GLsizeiptr size, const void* data, GLenum usage)
    {
        bind();
        glBufferData(GL_ARRAY_BUFFER, size, data, usage);
    }


    IBO::IBO() try
        :   id([&]()->GLuint {
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
            glDeleteBuffers(1, &id);
        }
        catch (...) {
            LOG_CTX_WRN(THIS_NAME, "Could not delete properly: no valid Window bound.");
            return;
        };
    }

    void IBO::bind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
    }

    void IBO::edit(GLsizeiptr size, const void* data, GLenum usage)
    {
        bind();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
    }
}
SSS_GL_END;