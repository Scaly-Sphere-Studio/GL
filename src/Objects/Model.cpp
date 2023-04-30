#include "GL/Objects/Model.hpp"
#include "GL/Globals.hpp"

SSS_GL_BEGIN;

ModelBase::ModelBase() try
{
    setScaling();
    setRotation();
    setTranslation();
}
CATCH_AND_RETHROW_METHOD_EXC;

ModelBase::~ModelBase() = default;

void ModelBase::setScaling(glm::vec3 scaling)
{
    _scaling = glm::scale(glm::mat4(1), scaling);
    _should_compute_mat4 = true;
}

void ModelBase::scale(glm::vec3 scaling)
{
    _scaling = glm::scale(_scaling, scaling);
    _should_compute_mat4 = true;
}

void ModelBase::setRotation(glm::vec3 angles)
{
    _rotation = glm::rotate(glm::mat4(1), glm::radians(0.f), glm::vec3(0, 0, -1));
    rotate(angles);
}

void ModelBase::rotate(glm::vec3 angles)
{
    if (angles.x != 0.f)
        _rotation = glm::rotate(_rotation, glm::radians(angles.x), glm::vec3(1, 0, 0));
    if (angles.y != 0.f)
        _rotation = glm::rotate(_rotation, glm::radians(angles.y), glm::vec3(0, 1, 0));
    if (angles.z != 0.f)
        _rotation = glm::rotate(_rotation, glm::radians(angles.z), glm::vec3(0, 0, 1));
    _should_compute_mat4 = true;
}

void ModelBase::setTranslation(glm::vec3 position)
{
    _translation = glm::translate(glm::mat4(1), position);
    _should_compute_mat4 = true;
}

void ModelBase::translate(glm::vec3 translation)
{
    _translation = glm::translate(_translation, translation);
    _should_compute_mat4 = true;
}

glm::mat4 ModelBase::getModelMat4()
{
    if (_should_compute_mat4) {
        _model_mat4 = _translation * _rotation * _scaling;
        _should_compute_mat4 = false;
    }
    return _model_mat4;
}

void ModelBase::getAllTransformations(glm::vec3 & scaling, glm::vec3 & rot_angles, glm::vec3 & translation)
{
    // Rotation as quaternion
    glm::quat rotation;
    // Unused parameters
    glm::vec3 skew;
    glm::vec4 perspective;
    // Decompose ModelBase matrix
    glm::decompose(getModelMat4(), scaling, rotation, translation, skew, perspective);
    // Get euler angles (in degrees) from quaternion
    rot_angles = glm::degrees(glm::eulerAngles(rotation));
}

glm::vec3 ModelBase::getScaling()
{
    glm::vec3 scaling, rot_angles, translation;
    getAllTransformations(scaling, rot_angles, translation);
    return scaling;
}

glm::vec3 ModelBase::getRotation()
{
    glm::vec3 scaling, rot_angles, translation;
    getAllTransformations(scaling, rot_angles, translation);
    return rot_angles;
}

glm::vec3 ModelBase::getTranslation()
{
    glm::vec3 scaling, rot_angles, translation;
    getAllTransformations(scaling, rot_angles, translation);
    return translation;
}

bool ModelBase::isHovered() const noexcept
{
    return getHoveredModel().get() == this;
}

bool ModelBase::isClicked() const noexcept
{
    return getClickedModel().get() == this;
}

bool ModelBase::isHeld() const noexcept
{
    return getHeldModel().get() == this;
}

SSS_GL_END;