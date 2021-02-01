#include "SSS/GL/Plane.hpp"
#include "SSS/GL/Window.hpp"

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
    _texture.edit(data.get(), w, h);
    // Scale to keep texture ratio
    _updateTexScaling(w, h);
}
__CATCH_AND_RETHROW_METHOD_EXC

// Creates a Plane model and returns a unique_ptr
Plane::Shared Plane::create() try
{
    // Use new instead of std::make_shared to access private constructor
    return std::dynamic_pointer_cast<Plane>(_instances.emplace_back(Shared(new Plane())));
}
__CATCH_AND_RETHROW_FUNC_EXC

// Creates a Plane model and returns a unique_ptr
Plane::Shared Plane::create(std::string const& filepath) try
{
    // Use new instead of std::make_shared to access private constructor
    return std::dynamic_pointer_cast<Plane>(_instances.emplace_back(Shared(new Plane(filepath))));
}
__CATCH_AND_RETHROW_FUNC_EXC

void Plane::unload(Shared instance)
{
    Model::unload(instance);
}

void Plane::unloadAll()
{
    for (auto it = _instances.cbegin(); it != _instances.cend(); ++it) {
        Shared is_plane(std::dynamic_pointer_cast<Plane>(*it));
        if (is_plane) {
            _instances.erase(it);
        }
    }
}

void Plane::editTexture(const GLvoid* pixels, GLsizei width, GLsizei height,
    GLenum format, GLint internalformat, GLenum type, GLint level)
{
    _texture.edit(pixels, width, height, format, internalformat, type, level);
    _updateTexScaling(width, height);
}

glm::mat4 Plane::getModelMat4() noexcept
{
    if (_should_compute_mat4) {
        glm::mat4 scaling = glm::scale(_scaling, _tex_scaling * _win_scaling);
        _model_mat4 = _translation * _rotation * scaling;
        _should_compute_mat4 = false;
    }
    return _model_mat4;
}

void Plane::draw() const
{
    _static_vao->bind();
    _texture.bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

// Updates the scaling to handle the new screen ratio
void Plane::_updateScreenRatio() try
{
    for (Model::Shared const& model : _instances) {
        Shared const plane = std::dynamic_pointer_cast<Plane>(model);
        if (plane) {
            plane->_updateWinScaling();
        }
    }
}
__CATCH_AND_RETHROW_FUNC_EXC

void Plane::_updateTexScaling(int width, int height)
{
    if (_tex_w == width && height == _tex_h) {
        return;
    }
    _tex_w = width;
    _tex_h = height;

    float const ratio = (static_cast<float>(_tex_w) / static_cast<float>(_tex_h));
    glm::vec3 scaling(1);
    if (ratio < 1.f) {
        scaling[0] = ratio;
    }
    else {
        scaling[1] = 1 / ratio;
    }

    if (_tex_scaling != scaling) {
        _tex_scaling = scaling;
        _should_compute_mat4 = true;
        _updateWinScaling();
    }
}

void Plane::_updateWinScaling()
{
    Window::Shared const win = Window::get(_texture.context);
    if (!win) {
        return;
    }

    float const ratio = (static_cast<float>(_tex_w) / static_cast<float>(_tex_h));
    float const screen_ratio = win->getScreenRatio();
    glm::vec3 scaling(1);
    if (ratio < 1.f) {
        if (screen_ratio < ratio) {
            scaling[0] = 1 / ratio;
            scaling[1] = screen_ratio / ratio;
        }
        else {
            scaling[0] = 1 / screen_ratio;
        }
    }
    else {
        if (screen_ratio > ratio) {
            scaling[1] = ratio;
            scaling[0] = ratio / screen_ratio;
        }
        else {
            scaling[1] = screen_ratio;
        }
    }
    if (_win_scaling != scaling) {
        _win_scaling = scaling;
        _should_compute_mat4 = true;
    }
}

__SSS_GL_END
