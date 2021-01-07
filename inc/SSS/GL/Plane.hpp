#pragma once

#include "Model.hpp"

__SSS_GL_BEGIN

class Plane : public Model {
private:
    static VAO::Shared _static_vao;
    static VBO::Shared _static_vbo;
    static IBO::Shared _static_ibo;
    static void _init_statics();

public:
    Plane();
    Plane(std::string const& filepath);
    ~Plane();

    void editTexture(const GLvoid* pixels, GLsizei width, GLsizei height,
        GLenum format = GL_RGBA, GLint internalformat = GL_RGBA,
        GLenum type = GL_UNSIGNED_BYTE, GLint level = 0);

    void draw() const;

private:
    Texture _texture;
    void _scaleToTextureRatio(int width, int height);
};

__SSS_GL_END