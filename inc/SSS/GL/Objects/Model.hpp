#ifndef SSS_GL_MODEL_HPP
#define SSS_GL_MODEL_HPP

#include "SSS/GL/Objects/Basic.hpp"
#include <glm/gtx/matrix_decompose.hpp>

/** @file
 *  Defines abstract class SSS::GL::ModelBase.
 */

SSS_GL_BEGIN;

/** Abstract base class for %Model matrix.
 *  @sa Plane
 */
class ModelBase {
public:
    ModelBase();
    /** Virtual destructor, default.*/
    virtual ~ModelBase() = 0;

    using Shared = std::shared_ptr<ModelBase>;
    using Weak = std::weak_ptr<ModelBase>;

    /** Sets scaling (default: 1*1*1).*/
    void setScaling(glm::vec3 scaling = glm::vec3(1.f));
    /** Sets rotation angles (default: 0*0*0).*/
    void setRotation(glm::vec3 angles = glm::vec3(0.f));
    /** Sets translation (default: 0*0*0).*/
    void setTranslation(glm::vec3 position = glm::vec3(0.f));

    /** Modifies scaling.*/
    void scale(glm::vec3 scaling);
    inline void scale(float scaling) { scale(glm::vec3(scaling)); };
    /** Modifies rotation angles.*/
    void rotate(glm::vec3 angles);
    /** Modifies translation.*/
    void translate(glm::vec3 translation);

    /** Returns the %Model matrix, compute if needed.*/
    virtual glm::mat4 getModelMat4();
    /** Returns the scaling, rotation angles, and translation in given parameters.*/
    virtual void getAllTransformations(glm::vec3& scaling, glm::vec3& rot_angles,
        glm::vec3& translation);
    glm::vec3 getScaling();
    glm::vec3 getRotation();
    glm::vec3 getTranslation();

    virtual bool isHovered() const noexcept = 0;
    virtual bool isClicked() const noexcept = 0;
    virtual bool isHeld() const noexcept = 0;

protected:
    bool isHovered(std::shared_ptr<Window> window) const noexcept;
    bool isClicked(std::shared_ptr<Window> window) const noexcept;
    bool isHeld(std::shared_ptr<Window> window) const noexcept;

    /** Scaling part of the %Model matrix.*/
    glm::mat4 _scaling;
    /** Rotation part of the %Model matrix.*/
    glm::mat4 _rotation;
    /** Translation part of the %Model matrix.*/
    glm::mat4 _translation;
    /** %Model matrix, computed by getModelMat4().*/
    glm::mat4 _model_mat4;
    /** Whether getModelMat4() should compute _model_mat4.*/
    bool _should_compute_mat4{ true };
};

template<class Derived>
class Model : public ModelBase, public _internal::InstancedWindowObject<Derived> {
protected:
    Model(std::shared_ptr<Window> window)
        : ModelBase(), _internal::InstancedWindowObject<Derived>(window) {}

public:
    using _internal::SharedWindowObject<Derived>::Shared;
    using _internal::SharedWindowObject<Derived>::Weak;

    virtual bool isHovered() const noexcept {
        return ModelBase::isHovered(_internal::WindowObject::getWindow());
    };
    virtual bool isClicked() const noexcept {
        return ModelBase::isClicked(_internal::WindowObject::getWindow());
    };
    virtual bool isHeld() const noexcept {
        return ModelBase::isHeld(_internal::WindowObject::getWindow());
    };
};


SSS_GL_END;

#endif // SSS_GL_MODEL_HPP