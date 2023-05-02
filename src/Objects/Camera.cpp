#include "GL/Objects/Camera.hpp"
#include "GL/Window.hpp"

SSS_GL_BEGIN;

Camera::Camera()
{
    _computeView();
    _computeProjections();
}

Camera::~Camera()
{
}

void Camera::setPosition(glm::vec3 position)
{
    _position = position;
    _computeView();
}

// This is a simplified roll pitch yaw function where no roll is computed
static glm::vec3 yaw_pitch(glm::vec3 ray, glm::vec2 angles)
{
    glm::mat3 mat;

    // No roll operation means alpha=0
    float const beta = glm::radians(-angles.y);
    float const gamma = glm::radians(angles.x);

    // cos(0)=1, sin(0)=0
    float const cos_beta = cosf(beta);
    float const sin_beta = sinf(beta);
    float const cos_gamma = cosf(gamma);
    float const sin_gamma = sinf(gamma);

    // Simplified 1 and 0 multiplications
    mat[0][0] = cos_beta;
    mat[0][1] = sin_beta * sin_gamma;
    mat[0][2] = sin_beta * cos_gamma;
    mat[1][0] = 0;
    mat[1][1] = cos_gamma;
    mat[1][2] = -sin_gamma;
    mat[2][0] = -sin_beta;
    mat[2][1] = cos_beta * sin_gamma;
    mat[2][2] = cos_beta * cos_gamma;

    return (ray * mat);
}

void Camera::move(glm::vec3 translation, bool use_rotation_axis)
{
    if (use_rotation_axis) {
        translation = yaw_pitch(translation, _rot_angles);
    }
    _position += translation;
    _computeView();
}

static void limitAngles(glm::vec2& angles)
{
    if (angles.x >= 90.f)
        angles.x = 89.99f;
    if (angles.x <= -90.f)
        angles.x = -89.99f;
    while (angles.y >= 360.f)
        angles.y -= 360.f;
    while (angles.y <= -360.f)
        angles.y += 360.f;
}

void Camera::setRotation(glm::vec2 angles)
{
    _rot_angles = angles;
    limitAngles(_rot_angles);
    _computeView();
}

void Camera::rotate(glm::vec2 angles)
{
    _rot_angles += angles;
    limitAngles(_rot_angles);
    _computeView();
}

void Camera::setProjectionType(Projection type)
{
    _projection_type = type;
    _computeProjections();
}

void Camera::setFOV(float degrees)
{
    _fov = degrees;
    _computeProjections();
}

void Camera::setRange(float z_near, float z_far)
{
    _z_near = z_near;
    _z_far = z_far;
    _computeProjections();
}

void Camera::setZNear(float z_near)
{
    _z_near = z_near;
    _computeProjections();
}

void Camera::setZFar(float z_far)
{
    _z_far = z_far;
    _computeProjections();
}

void Camera::_computeView()
{
    static constexpr glm::vec3 up_vec(0, 1, 0);

    glm::vec3 direction = yaw_pitch(glm::vec3(0, 0, -1), _rot_angles);
    glm::vec3 const relative_center = _position + direction;
    _view = glm::lookAt(_position, relative_center, up_vec);
    _computeVP();
}

glm::mat4 Camera::_computeProjection(Window const& window)
{
    glm::mat4 projection;

    float const ratio = window.getRatio();

    switch (_projection_type) {
    case Projection::Ortho: {
        float const x = ratio > 1.f ? ratio : 1.f;
        float const y = ratio > 1.f ? 1.f : 1.f / ratio;
        projection = glm::ortho(-x, x, -y, y, _z_near, _z_far);
    }
        break;
    case Projection::OrthoFixed: {
        int w, h;
        window.getDimensions(w, h);
        float const x = static_cast<float>(w) / 2.f;
        float const y = static_cast<float>(h) / 2.f;
        projection = glm::ortho(-x, x, -y, y, _z_near, _z_far);
    }
        break;
    case Projection::Perspective:
        projection = glm::perspective(glm::radians(_fov), ratio, _z_near, _z_far);
        break;
    }

    return projection;
}

void Camera::_computeProjections()
{
    std::vector<Window*> all_windows = Window::getAll();
    for (Window const* window : all_windows)
        _projections[window->getGLFWwindow()] = _computeProjection(*window);
    _computeVP();
}

void Camera::_computeVP()
{
    for (auto const& [ptr, projection] : _projections)
        _vps[ptr] = projection * _view;
}

SSS_GL_END;