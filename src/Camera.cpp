#include "SSS/GL/Camera.hpp"
#include "SSS/GL/Window.hpp"

__SSS_GL_BEGIN

Camera::Camera(std::weak_ptr<Window> weak_window)
    : _internal::WindowObject(weak_window)
{
    Window::Shared const window = _window.lock();
    if (!window) {
        return;
    }
    _screen_ratio = window->getScreenRatio();
    _computeView();
    _computeProjection();
}

void Camera::setPosition(glm::vec3 position)
{
    static constexpr glm::mat4 center_mat(0.f, 0.f, 0.f, 0.f,
        0.f, 0.f, 0.f, 0.f,
        0.f, 0.f, 0.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    );
    _position = glm::translate(center_mat, position);
    _computeView();
}

void Camera::move(glm::vec3 translation)
{
    _position = glm::translate(_position, translation);
    _computeView();
}

void Camera::setRotation(float radians, glm::vec3 axe)
{
    _rotation = glm::rotate(glm::mat4(1), radians, axe);
    _computeView();
}

void Camera::rotate(float radians, glm::vec3 axe)
{
    _rotation = glm::rotate(_rotation, radians, axe);
    glm::vec3 v = _rotation * glm::vec4(1);
    _computeView();
}

void Camera::setProjectionType(Projection type)
{
    _projection_type = type;
    _computeProjection();
}

void Camera::setFOV(float radians)
{
    _fov = radians;
    _computeProjection();
}

void Camera::setRange(float z_near, float z_far)
{
    _z_near = z_near;
    _z_far = z_far;
    _computeProjection();
}

glm::mat4 Camera::getMVP(glm::mat4 model) const
{
    return _projection * _view * model;
}

void Camera::_computeView()
{
    static constexpr glm::vec3 up_vec(0, 1, 0);

    glm::vec3 const eye = _position * glm::vec4(1);
    glm::vec3 const axis = glm::normalize(_rotation * glm::vec4(1));
    __LOG_FUNC_MSG(toString(axis[0]) + "/" + toString(axis[1]) + "/" + toString(axis[2]));
    glm::vec3 const center = glm::translate(_position, axis) * glm::vec4(1);
    __LOG_FUNC_MSG(toString(center[0]) + "/" + toString(center[1]) + "/" + toString(center[2]));
    _view = glm::lookAt(eye, center, glm::vec3(0, 1, 0));
}

void Camera::_computeProjection()
{
    switch (_projection_type) {
    case Projection::Ortho: {
        float const x = _screen_ratio > 1.f ? _screen_ratio : 1.f;
        float const y = _screen_ratio > 1.f ? 1.f : 1.f / _screen_ratio;
        _projection = glm::ortho(-x, x, -y, y, _z_near, _z_far);
    }
                          break;
    case Projection::Perspective:
        _projection = glm::perspective(_fov, _screen_ratio, _z_near, _z_far);
        break;
    }
}

__SSS_GL_END