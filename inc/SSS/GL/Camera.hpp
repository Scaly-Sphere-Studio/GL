#pragma once

#include "_internal/pointers.hpp"
#include "_internal/callbacks.hpp"

__SSS_GL_BEGIN

class Camera : _internal::WindowObject {
    friend void _internal::window_resize_callback(GLFWwindow* ptr, int w, int h);
    friend class Window;

private:
    Camera(std::weak_ptr<Window> window);           // Constructor
public:
    // Rule of 5
    Camera()                            = delete;   // Constructor (deleted)
    ~Camera()                           = default;  // Destructor
    Camera(const Camera&)               = delete;   // Copy constructor
    Camera(Camera&&)                    = delete;   // Move constructor
    Camera& operator=(const Camera&)    = delete;   // Copy assignment
    Camera& operator=(Camera&&)         = delete;   // Move assignment

    using Ptr = std::unique_ptr<Camera>;

    void setPosition(glm::vec3 position);
    void move(glm::vec3 translation, bool use_rotation_axis = true);
    void setRotation(float radians, glm::vec3 axis);
    void rotate(float radians, glm::vec3 axis);

    enum class Projection {
        Ortho,
        Perspective
    };
    void setProjectionType(Projection type);
    void setFOV(float radians);
    void setRange(float z_near, float z_far);

    glm::mat4 getMVP(glm::mat4 model) const;

private:
    glm::mat4 _position{ glm::translate(glm::mat4(1), glm::vec3(-1)) };
    glm::quat _rotation{ 1, 0, 0, 0 };
    glm::mat4 _view{ 1 };
    void _computeView();

    float _screen_ratio{ 1.f };
    float _fov{ glm::radians(70.f) };
    float _z_near{ 0.1f }, _z_far{ 100.f };
    Projection _projection_type{ Projection::Ortho };
    glm::mat4 _projection{ 1 };
    void _computeProjection();
};

__SSS_GL_END