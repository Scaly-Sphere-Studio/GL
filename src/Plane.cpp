#include "SSS/GL/Plane.hpp"

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
{
    // Init statics
    _init_statics();
    _vao = _static_vao;
    _vbo = _static_vbo;
    _ibo = _static_ibo;
}
__CATCH_AND_RETHROW_METHOD_EXC

Plane::Plane(TextureBase::Shared texture) try
    : Plane()
{
    useTexture(texture);
}
__CATCH_AND_RETHROW_METHOD_EXC

void Plane::useTexture(TextureBase::Shared texture)
{
    _texture.swap(texture);
    _updateTexScaling();
}

glm::mat4 Plane::getModelMat4() noexcept
{
    if (_should_compute_mat4) {
        glm::mat4 scaling = glm::scale(_scaling, _tex_scaling);
        _model_mat4 = _translation * _rotation * scaling;
        _should_compute_mat4 = false;
    }
    return _model_mat4;
}

void Plane::draw() const
{
    _static_vao->bind();
    _texture->bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

void Plane::_updateTexScaling()
{
    // Check if dimensions changed
    int w, h;
    _texture->getDimensions(w, h);
    if (_tex_w == w && _tex_h == h) {
        return;
    }
    // Update dimensions
    _tex_w = w;
    _tex_h = h;

    // Get texture ratio
    float const ratio = (static_cast<float>(_tex_w) / static_cast<float>(_tex_h));
    // Calculate according scaling
    glm::vec3 scaling(1);
    if (ratio < 1.f) {
        scaling[1] = 1 / ratio;
    }
    else {
        scaling[0] = ratio;
    }
    __LOG_METHOD_MSG(context_msg("scaling", toString(scaling[0])) + "x" + toString(scaling[1]))

    // If scaling changed, indicate that the Model matrice should be computed
    if (_tex_scaling != scaling) {
        _tex_scaling = scaling;
        _should_compute_mat4 = true;
    }
}

__SSS_GL_END
