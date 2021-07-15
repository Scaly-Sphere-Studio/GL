#include "SSS/GL/Model.hpp"
#include "SSS/GL/Window.hpp"

__SSS_GL_BEGIN

std::vector<Model::Weak> Model::_instances{};

Model::Model(std::shared_ptr<Window> window) try
    : _internal::WindowObject(window)
{
    _vao.reset(new VAO(window));
    _vbo.reset(new VBO(window));
    _ibo.reset(new IBO(window));
    resetTransformations(Transformation::All);
}
__CATCH_AND_RETHROW_METHOD_EXC

Model::~Model()
{
    cleanWeakPtrVector(_instances);
}

Model::Shared Model::create(std::shared_ptr<Window> window)
{
    // Use new instead of std::make_shared to access private constructor
    return (Shared)_instances.emplace_back(Model::Shared(new Model(window)));
}

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