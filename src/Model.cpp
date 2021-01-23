#include "SSS/GL/Model.hpp"

__SSS_GL_BEGIN

std::vector<Model::Shared> Model::_instances{};

Model::Model() try
{
    _vao.reset(new VAO);
    _vbo.reset(new VBO);
    _ibo.reset(new IBO);
    resetTransformations(Transformation::All);
}
__CATCH_AND_RETHROW_METHOD_EXC

Model::Shared Model::create() try
{
    // Use new instead of std::make_shared to access private constructor
    return Shared(_instances.emplace_back(Shared(new Model())));
}
__CATCH_AND_RETHROW_FUNC_EXC

void Model::unload(Shared instance) try
{
    for (auto it = _instances.cbegin(); it != _instances.cend(); ++it) {
        if (*it == instance) {
            _instances.erase(it);
            break;
        }
    }
}
__CATCH_AND_RETHROW_FUNC_EXC

void Model::unloadAll() noexcept
{
    _instances.clear();
}

void Model::scale(glm::vec3 scaling)
{
    _scaling = glm::scale(_scaling, scaling);
    _should_compute_mat4 = true;
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