#include "SSS/GL/Model.hpp"
#include "SSS/GL/Window.hpp"

__SSS_GL_BEGIN

Model::Model(std::weak_ptr<Window> window) try
    : _internal::WindowObject(window)
{
    Context const context(_window);
    _vao.reset(new VAO(_window));
    _vbo.reset(new VBO(_window));
    _ibo.reset(new IBO(_window));
    resetTransformations(Transformation::All);
}
__CATCH_AND_RETHROW_METHOD_EXC

Model::~Model()
{
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

glm::mat4 Model::getModelMat4()
{
    if (_should_compute_mat4) {
        _model_mat4 = _translation * _rotation * _scaling;
        _should_compute_mat4 = false;
    }
    return _model_mat4;
}

void Model::useCamera(uint32_t camera_id) noexcept
{
    _camera_id = camera_id;
    _use_camera = true;
}

glm::mat4 Model::getMVP()
{
    // If anything prevents the use of the Camera to compute
    // the MVP matrix, the Model matrix will be returned instead

    // Retrieve Model matrix
    glm::mat4 const model_mat4 = getModelMat4();
    // Ensure a camera is sets
    if (!_use_camera) {
        return model_mat4;
    }
    // Retrieve Window and ensure it still exists
    Window::Shared const window = _window.lock();
    if (!window) {
        return model_mat4;
    }
    // Ensure the Window holds a Camera of given ID
    Window::Objects const& objects = window->getObjects();
    if (objects.cameras.count(_camera_id) == 0) {
        return model_mat4;
    }
    // Ensure the Camera of given ID still exists
    Camera::Ptr const& camera = objects.cameras.at(_camera_id);
    if (!camera) {
        return model_mat4;
    }
    // Compute MVP via Camera
    return camera->getMVP(model_mat4);
}

__SSS_GL_END