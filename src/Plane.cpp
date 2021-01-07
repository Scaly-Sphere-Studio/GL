#include "SSS/GL/Plane.hpp"

// Init STB
#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include <stb_image.h>

__SSS_GL_BEGIN

// Init static members
VAO::Shared Plane::_static_vao{ nullptr };
VBO::Shared Plane::_static_vbo{ nullptr };
IBO::Shared Plane::_static_ibo{ nullptr };

void Plane::_init_statics() try
{
    if (_static_vao && _static_vbo && _static_ibo) {
        return;
    }

    _static_vao.reset(new VAO);
    _static_vbo.reset(new VBO);
    _static_ibo.reset(new IBO);

    _static_vao->bind();
    _static_vbo->bind();
    _static_ibo->bind();

    constexpr float vertices[] = {
        // positions          // texture coords (1 - y)
        -0.5f,  0.5f, 0.0f,   0.0f, 1.f - 1.0f,   // top left
        -0.5f, -0.5f, 0.0f,   0.0f, 1.f - 0.0f,   // bottom left
         0.5f, -0.5f, 0.0f,   1.0f, 1.f - 0.0f,   // bottom right
         0.5f,  0.5f, 0.0f,   1.0f, 1.f - 1.0f    // top right
    };
    constexpr unsigned int indices[] = {
        0, 1, 2,  // first triangle
        0, 2, 3   // second triangle
    };

    _static_vbo->edit(sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    _static_ibo->edit(sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}
__CATCH_AND_RETHROW_FUNC_EXC

Plane::Plane() try
    : _texture(GL_TEXTURE_2D)
{
    // Init statics
    _init_statics();
    _vao = _static_vao;
    _vbo = _static_vbo;
    _ibo = _static_ibo;

    _texture.bind();
    _texture.parameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    _texture.parameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    _texture.parameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    _texture.parameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
}
__CATCH_AND_RETHROW_METHOD_EXC

Plane::Plane(std::string const& filepath) try
    : Plane()
{
    int w, h;
    SSS::C_Ptr<unsigned char, void(*)(void*), stbi_image_free> data
        = stbi_load(filepath.c_str(), &w, &h, nullptr, 4);
    // Throw if error
    if (!data) {
        SSS::throw_exc(stbi_failure_reason());
    }
    // Give the image to the OpenGL texture
    _texture.edit(0, GL_RGBA, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data.get());
    // Scale to keep texture ratio
    _scaleToTextureRatio(w, h);
}
__CATCH_AND_RETHROW_METHOD_EXC

Plane::~Plane()
{
}

void Plane::editTexture(const GLvoid* pixels, GLsizei width, GLsizei height,
    GLenum format, GLint internalformat, GLenum type, GLint level)
{
    _texture.edit(level, internalformat, width, height, format, type, pixels);
    _scaleToTextureRatio(width, height);
}

void Plane::draw() const
{
    _static_vao->bind();
    _texture.bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

void Plane::_scaleToTextureRatio(int width, int height)
{
    // Scale to keep texture ratio
    glm::vec3 scaling(1);
    if (width > height) {
        float const ratio = (static_cast<float>(height) / static_cast<float>(width));
        scaling[0] /= ratio;
    }
    else if (height > width) {
        float const ratio = (static_cast<float>(width) / static_cast<float>(height));
        scaling[1] /= ratio;
    }
    glm::mat4 const new_scaling = glm::scale(glm::mat4(1), scaling);
    if (_og_scaling != new_scaling) {
        _og_scaling = new_scaling;
        resetTransformations(Transformation::Scaling);
    }
}

__SSS_GL_END