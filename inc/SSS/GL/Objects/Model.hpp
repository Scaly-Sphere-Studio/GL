#ifndef SSS_GL_MODEL_HPP
#define SSS_GL_MODEL_HPP

#include "SSS/GL/Objects/Basic.hpp"
#include <glm/gtx/matrix_decompose.hpp>

/** @file
 *  Defines template abstract class SSS::GL::Model.
 */

SSS_GL_BEGIN;

/** Template abstract base class for %Model matrix.
 *  @sa Plane
 */
class Model {
public:
    Model();
    /** Virtual destructor, default.*/
    virtual ~Model() = 0;

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

protected:
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

SSS_GL_END;

#endif // SSS_GL_MODEL_HPP