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
    _position = position;
    _computeView();
}

void Camera::move(glm::vec3 translation, bool use_rotation_axis)
{
    if (use_rotation_axis) {
        translation = glm::rotate(_rotation, translation);
    }
    _position += translation;
    _computeView();
}

void Camera::setRotation(float radians, glm::vec3 axis)
{
    _rotation = glm::quat(glm::angleAxis(radians, glm::normalize(axis)));
    _computeView();
}

void Camera::rotate(float radians, glm::vec3 axis)
{
    _rotation *= glm::quat(glm::angleAxis(radians, glm::normalize(axis)));
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
    static constexpr glm::vec3 center_base(0, 0, -1);

    glm::vec3 const relative_center = _position + glm::rotate(_rotation, center_base);
    _view = glm::lookAt(_position, relative_center, up_vec);
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