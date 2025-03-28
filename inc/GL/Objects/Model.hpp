#ifndef SSS_GL_MODEL_HPP
#define SSS_GL_MODEL_HPP

#include "Basic.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

/** @file
 *  Defines abstract class SSS::GL::ModelBase.
 */

SSS_GL_BEGIN;

// Ignore warning about STL exports as they're private members
#pragma warning(push, 2)
#pragma warning(disable: 4251)
#pragma warning(disable: 4275)

/** Abstract base class for %Model matrix.
 *  @sa Plane
 */
class SSS_GL_API ModelBase : public Subject {
public:
    ModelBase();
    /** Virtual destructor, default.*/
    virtual ~ModelBase() = 0;

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

protected:
    void _computeModelMat4();
    virtual inline glm::mat4 _getScalingMat4() const { return _scaling; };
    virtual inline glm::mat4 _getRotationMat4() const { return _rotation; };
    virtual inline glm::mat4 _getTranslationMat4() const { return _translation; };

public:
    /** Returns the %Model matrix.*/
    inline glm::mat4 getModelMat4() const noexcept { return _model_mat4; };
    /** Returns the scaling, rotation angles, and translation in given parameters.*/
    virtual void getAllTransformations(glm::vec3& scaling, glm::vec3& rot_angles,
        glm::vec3& translation) const;
    glm::vec3 getScaling();
    glm::vec3 getRotation();
    glm::vec3 getTranslation();

    bool isHovered() const noexcept;
    bool isClicked() const noexcept;
    bool isHeld() const noexcept;

private:
    /** Scaling part of the %Model matrix.*/
    glm::mat4 _scaling;
    /** Rotation part of the %Model matrix.*/
    glm::mat4 _rotation;
    /** Translation part of the %Model matrix.*/
    glm::mat4 _translation;
    /** %Model matrix, computed by getModelMat4().*/
    glm::mat4 _model_mat4;
};

#pragma warning(pop)

SSS_GL_END;

#endif // SSS_GL_MODEL_HPP