#include "SSS/GL/Model.hpp"
#include "SSS/GL/Window.hpp"
#include <glm/gtx/matrix_decompose.hpp>

std::map<uint32_t, SSS::GL::Model::OnClickFunc> SSS::GL::Model::on_click_funcs {
    { 0, nullptr }
};
std::map<uint32_t, SSS::GL::Model::PassiveFunc> SSS::GL::Model::passive_funcs {
    { 0, nullptr }
};

__SSS_GL_BEGIN;

Model::Model(std::weak_ptr<Window> window) try
    : _internal::WindowObject(window)
{
    setScaling();
    setRotation();
    setTranslation();
}
__CATCH_AND_RETHROW_METHOD_EXC

Model::~Model()
{
}

void Model::setScaling(glm::vec3 scaling)
{
    _scaling = glm::scale(glm::mat4(1), scaling);
    _should_compute_mat4 = true;
}

void Model::setScaling(float scaling)
{
    setScaling(glm::vec3(scaling));
}

void Model::scale(glm::vec3 scaling)
{
    _scaling = glm::scale(_scaling, scaling);
    _should_compute_mat4 = true;
}

void Model::scale(float scaling)
{
    scale(glm::vec3(scaling));
}

void Model::setRotation(glm::vec3 angles)
{
    _rotation = glm::rotate(glm::mat4(1), glm::radians(0.f), glm::vec3(0, 0, -1));
    rotate(angles);
}

void Model::rotate(glm::vec3 angles)
{
    if (angles.x != 0.f)
        _rotation = glm::rotate(_rotation, glm::radians(angles.x), glm::vec3(1, 0, 0));
    if (angles.y != 0.f)
        _rotation = glm::rotate(_rotation, glm::radians(angles.y), glm::vec3(0, 1, 0));
    if (angles.z != 0.f)
        _rotation = glm::rotate(_rotation, glm::radians(angles.z), glm::vec3(0, 0, 1));
    _should_compute_mat4 = true;
}

void Model::setTranslation(glm::vec3 position)
{
    _translation = glm::translate(glm::mat4(1), position);
    _should_compute_mat4 = true;
}

void Model::translate(glm::vec3 translation)
{
    _translation = glm::translate(_translation, translation);
    _should_compute_mat4 = true;
}

glm::mat4 Model::getModelMat4()
{
    if (_should_compute_mat4) {
        _model_mat4 = _translation * _rotation * _scaling;
        _should_compute_mat4 = false;
    }
    return _model_mat4;
}

void Model::getAllTransformations(glm::vec3& scaling, glm::vec3& rot_angles, glm::vec3& translation)
{
    // Rotation as quaternion
    glm::quat rotation;
    // Unused parameters
    glm::vec3 skew;
    glm::vec4 perspective;
    // Decompose Model matrix
    glm::decompose(getModelMat4(), scaling, rotation, translation, skew, perspective);
    // Get euler angles (in degrees) from quaternion
    rot_angles = glm::degrees(glm::eulerAngles(rotation));
}

// Called whenever the button is clicked (determined by Hitbox).
void Model::_callOnClickFunction(GLFWwindow* ptr, uint32_t id, int button, int action, int mods) try
{
    if (on_click_funcs.count(_on_click_func_id) == 0) {
        return;
    }
    OnClickFunc const f = on_click_funcs.at(_on_click_func_id);
    if (f != nullptr) {
        f(ptr, id, button, action, mods);
    }
}
__CATCH_AND_RETHROW_METHOD_EXC

void Model::_callPassiveFunction(GLFWwindow* ptr, uint32_t id)
{
    if (passive_funcs.count(_passive_func_id) == 0) {
        return;
    }
    PassiveFunc const f = passive_funcs.at(_passive_func_id);
    if (f != nullptr) {
        f(ptr, id);
    }
}

__SSS_GL_END;