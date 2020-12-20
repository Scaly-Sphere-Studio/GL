#include "SSS/GL/Plane.hpp"

// Init STB
#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include <stb_image.h>

__SSS_GL_BEGIN

// Init static members
VAO::Ptr Plane::_vao{nullptr};
VBO::Ptr Plane::_vbo{nullptr};
IBO::Ptr Plane::_ibo{nullptr};

Plane::Plane() try
    : _texture(GL_TEXTURE_2D)
{
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
    glm::vec3 scaling(1);
    if (w > h) {
        float const ratio = (static_cast<float>(h) / static_cast<float>(w));
        scaling[0] /= ratio;
    }
    else if (h > w) {
        float const ratio = (static_cast<float>(w) / static_cast<float>(h));
        scaling[1] /= ratio;
    }
    _og_scaling = glm::scale(glm::mat4(1), scaling);
    resetTransformations(Transformation::Scaling);
}
__CATCH_AND_RETHROW_METHOD_EXC

Plane::~Plane()
{
}

void Plane::init() try
{
    if (_vao && _vbo && _ibo) {
        __LOG_FUNC_WRN("Plane already init.");
        return;
    }

    stbi_set_flip_vertically_on_load(true);

    _vao.reset(new VAO);
    _vbo.reset(new VBO);
    _ibo.reset(new IBO);

    _vao->bind();
    _vbo->bind();
    _ibo->bind();

    constexpr float vertices[] = {
        // positions          // texture coords
        -0.5f,  0.5f, 0.0f,   0.0f, 1.0f,   // top left
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,   // bottom left
         0.5f, -0.5f, 0.0f,   1.0f, 0.0f,   // bottom right
         0.5f,  0.5f, 0.0f,   1.0f, 1.0f    // top right
    };
    constexpr unsigned int indices[] = {
        0, 1, 2,  // first triangle
        0, 2, 3   // second triangle
    };

    _vbo->edit(sizeof(vertices), vertices, GL_STATIC_DRAW);
    _ibo->edit(sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}
__CATCH_AND_RETHROW_FUNC_EXC

void Plane::scale(glm::vec3 scaling)
{
    _scaling = glm::scale(_scaling, scaling);
    _model = _translation * _rotation * _scaling;
}

void Plane::rotate(float radians, glm::vec3 axis)
{
    _rotation = glm::rotate(_rotation, radians, axis);
    _model = _translation * _rotation * _scaling;
}

void Plane::translate(glm::vec3 translation)
{
    _translation = glm::translate(_translation, translation);
    _model = _translation * _rotation * _scaling;
}

void Plane::resetTransformations(Transformation transformations)
{
    if ((transformations & Transformation::Scaling) != Transformation::None) {
        _scaling = _og_scaling;
    }
    if ((transformations & Transformation::Rotation) != Transformation::None) {
        _rotation = _og_rotation;
    }
    if ((transformations & Transformation::Translation) != Transformation::None) {
        _translation = _og_translation;
    }
    _model = _translation * _rotation * _scaling;
}

void Plane::draw() const
{
    _vao->bind();
    _texture.bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}
__SSS_GL_END