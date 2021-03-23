#include "SSS/GL/Model.hpp"

__SSS_GL_BEGIN

Model::Model() try
{
    _vao.reset(new VAO);
    _vbo.reset(new VBO);
    _ibo.reset(new IBO);
    resetTransformations(Transformation::All);
}
__CATCH_AND_RETHROW_METHOD_EXC

void Model::scale(glm::vec3 scaling)
{
    _scaling = glm::scale(_scaling, scaling);
    _should_compute_mat4 = true;
}

void Model::scale(float scaling)
{
    scale(glm::vec3(scaling));
}

void Model::rotate(float radians, glm::vec3 axis)
{
    _rotation = glm::rotate(_rotation, radians, axis);
    _should_compute_mat4 = true;
}

void Model::translate(glm::vec3 translation)
{
    _translation = glm::translate(_translation, translation);
    _should_compute_mat4 = true;
}

void Model::resetTransformations(Transformation transformations)
{
    if (!!(transformations & Transformation::Scaling)) {
        _scaling = glm::scale(glm::mat4(1), glm::vec3(1));
    }
    if (!!(transformations & Transformation::Rotation)) {
        _rotation = glm::rotate(glm::mat4(1.), glm::radians(0.f), glm::vec3(0., 0., -1.));
    }
    if (!!(transformations & Transformation::Translation)) {
        _translation = glm::translate(glm::mat4(1), glm::vec3(0));
    }
    _should_compute_mat4 = true;
}

glm::mat4 Model::getModelMat4() noexcept
{
    if (_should_compute_mat4) {
        _model_mat4 = _translation * _rotation * _scaling;
        _should_compute_mat4 = false;
    }
    return _model_mat4;
}

__SSS_GL_END