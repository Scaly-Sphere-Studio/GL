#pragma once

#include "_internal/pointers.hpp"

__SSS_GL_BEGIN;
__INTERNAL_BEGIN;
void window_resize_callback(GLFWwindow*, int, int); // Pre-definition
__INTERNAL_END;

class Camera : _internal::WindowObject {
    friend void _internal::window_resize_callback(GLFWwindow*, int, int);
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
    inline glm::vec3 getPosition() const noexcept { return _position; };

    void setRotation(glm::vec2 angles);
    void rotate(glm::vec2 angles);
    inline glm::vec2 getRotation() const noexcept { return _rot_angles; };

    enum class Projection {
        Ortho,
        Perspective
    };
    void setProjectionType(Projection type);
    inline Projection getProjectionType() const noexcept { return _projection_type; };
    void setFOV(float degrees);
    inline float getFOV() const noexcept { return _fov; };
    void setRange(float z_near, float z_far);
    inline void getRange(float& z_near, float& z_far) const noexcept { z_near = _z_near; z_far = _z_far; };

    glm::mat4 getMVP(glm::mat4 model) const;

    inline glm::mat4 getView() const noexcept { return _view; };
    inline glm::mat4 getProjection() const noexcept { return _projection; };

private:
    glm::vec3 _position{ 0 };
    glm::vec2 _rot_angles{ 0 };
    glm::mat4 _view{ 1 };
    void _computeView();

    float _screen_ratio{ 1.f };
    float _fov{ 70.f };
    float _z_near{ 0.1f }, _z_far{ 100.f };
    Projection _projection_type{ Projection::Ortho };
    glm::mat4 _projection{ 1 };
    void _computeProjection();
};

__SSS_GL_END;