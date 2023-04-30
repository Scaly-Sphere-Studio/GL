#ifndef SSS_GL_CAMERA_HPP
#define SSS_GL_CAMERA_HPP

#include "Basic.hpp"
#include <glm/glm.hpp>
#include <glm/ext.hpp>

/** @file
 *  Defines class SSS::GL::Camera.
 */

SSS_GL_BEGIN;

// Ignore warning about STL exports as they're private members
#pragma warning(push, 2)
#pragma warning(disable: 4251)
#pragma warning(disable: 4275)

/** Abstractization of View and Projection matrices, used in Renderer::Chunk.
 *  @sa create()
 */
class SSS_GL_API Camera final : public Basic::InstancedBase<Camera> {
    friend class Window;
    friend class Basic::SharedBase<Camera>;
private:
    Camera();
public:
    /** Destructor, default
     *  @sa Window::removeCamera()
     */
    ~Camera();
    /** \cond INTERNAL*/
    // Rule of 5
    Camera(const Camera&)               = delete;   // Copy constructor
    Camera(Camera&&)                    = delete;   // Move constructor
    Camera& operator=(const Camera&)    = delete;   // Copy assignment
    Camera& operator=(Camera&&)         = delete;   // Move assignment
    /** \endcond*/

    /** Sets the position coordinates of the camera.
     *  The default coordinates are (0, 0, 0).
     *  @sa move(), getPosition()
     */
    void setPosition(glm::vec3 position);
    /** Moves the position coordinates of the camera.
     * 
     *  The given \c translation vector is added to the position.
     * 
     *  If \c use_rotation_axis is set to \c true, the \c translation
     *  vector undergoes the same rotation as the camera before being
     *  computed, which is useful to move in first or third person.
     *
     *  @sa setPosition(), getPosition()
     */
    void move(glm::vec3 translation, bool use_rotation_axis);
    inline void move(glm::vec3 translation) { move(translation, true); };
    /** Returns the position coordinates of the camera.
     *  @sa setPosition(), move()
     */
    inline glm::vec3 getPosition() const noexcept { return _position; };

    /** Sets the rotation angles of the camera.
     * 
     *  The default angles are (0, 0), which points to negative Z axis.
     * 
     *  The angles' range is automatically restricted to [-90, +90]
     *  for the x axis, and [-360, +360] for the y axis.
     * 
     *  @sa rotate(), getRotation().
     */
    void setRotation(glm::vec2 angles);
    /** Rotates the rotation angles of the camera.
     *  The angles' range is automatically restricted to [-90, +90]
     *  for the x axis, and [-360, +360] for the y axis.
     *  @sa setRotation(), getRotation().
     */
    void rotate(glm::vec2 angles);
    /** Returns the rotation angles of the camera.
     *  @sa setRotation(), rotate()
     */
    inline glm::vec2 getRotation() const noexcept { return _rot_angles; };

    /** All available %Projection types of matrices.
     *  @sa setProjectionType(), getProjectionType()
     */
    enum class Projection {
        /** Orthographic projection (2D).*/
        Ortho,
        /** Orthographic projection (2D) with a fixed pixel size.*/
        OrthoFixed,
        /** Perspective projection (3D).*/
        Perspective
    };
    /** Sets the Projection of the camera.
     *  @sa getProjectionType()
     */
    void setProjectionType(Projection type);
    /** Returns the current Projection of the camera.
     *  @sa setProjectionType()
     */
    inline Projection getProjectionType() const noexcept { return _projection_type; };
    /** Sets the FOV (Field of View) of the camera (used in Projection::Perspective).
     *  @sa getFOV()
     */
    void setFOV(float degrees);
    /** Returns the FOV (Field of View) of the camera, in degrees.
     *  @sa setFOV()
     */
    inline float getFOV() const noexcept { return _fov; };

    /** Sets the visibility range of the camera.
     *  @sa getRange(), setZNear(), setZFar()
     */
    void setRange(float z_near, float z_far);
    /** Sets the z_near range of the camera.
     *  @sa getRange(), setRange(), setZFar()
     */
    void setZNear(float z_near);
    /** Sets the z_far range of the camera.
     *  @sa getRange(), setRange(), setZNear()
     */
    void setZFar(float z_far);
    /** Returns the visibility range of the camera in given parameters.
     *  @sa setRange(), getZNear(), getZFar()
     */
    inline void getRange(float& z_near, float& z_far) const noexcept
        { z_near = _z_near; z_far = _z_far; };
    /** Returns the z_near range of the camera.
     *  @sa setRange(), getRange(), getZFar()
     */
    inline float getZNear() const noexcept { return _z_near; };
    /** Returns the z_far range of the camera.
     *  @sa setRange(), getRange(), getZNear()
     */
    inline float getZFar() const noexcept { return _z_far; };

    /** Returns the View and %Projection matrices previously computed together.
     *  @sa getView(), getProjection()
     */
    inline glm::mat4 getVP() const noexcept { return _vp; };
    /** Returns the previously computed View matrix.
     *  @sa getVP(), getProjection()
     */
    inline glm::mat4 getView() const noexcept { return _view; };
    /** Returns the previously computed %Projection matrix.
     *  @sa getVP(), getView()
     */
    inline glm::mat4 getProjection() const noexcept { return _projection; };

private:
    glm::vec3 _position{ 0 };
    glm::vec2 _rot_angles{ 0 };
    glm::mat4 _view{ 1 };
    void _computeView();

    float _fov{ 70.f };
    float _z_near{ 0.1f }, _z_far{ 100.f };
    Projection _projection_type{ Projection::Ortho };
    glm::mat4 _projection{ 1 };
    void _computeProjection();

    glm::mat4 _vp{ 1 };
    void _computeVP();
};

#pragma warning(pop)

SSS_GL_END;

#endif // SSS_GL_CAMERA_HPP