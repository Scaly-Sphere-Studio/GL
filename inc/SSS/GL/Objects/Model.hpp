#pragma once

#include "SSS/GL/Objects/Basic.hpp"
/** \cond INCLUDE*/
#include <glm/gtx/matrix_decompose.hpp>
/** \endcond*/

/** @file
 *  Defines abstract class SSS::GL::Model.
 */

SSS_GL_BEGIN;

template <class T>
class Model : public _internal::WindowObjectWithID {
    friend class Window;

protected:
    Model(std::weak_ptr<Window> window, uint32_t id);

public:
    virtual ~Model() = 0;

    void setScaling(glm::vec3 scaling);
    void setScaling(float scaling = 1.f);
    void scale(glm::vec3 scaling);
    void scale(float scaling);

    void setRotation(glm::vec3 angles = glm::vec3(0.f));
    void rotate(glm::vec3 angles);

    void setTranslation(glm::vec3 position = glm::vec3(0.f));
    void translate(glm::vec3 translation);

    virtual glm::mat4 getModelMat4();
    virtual void getAllTransformations(glm::vec3& scaling, glm::vec3& rot_angles,
        glm::vec3& translation);
   
    // Function format used on click events. Follows glfw callback syntax,
    // with the exception of the uint32_t parameter which stands for the Plane ID
    using OnClickFunc = void(*)(std::shared_ptr<Window>,
        std::unique_ptr<T> const&, int, int, int);
    // Sets the function (referenced by ID) to be called when the Plane is clicked.
    inline void setOnClickFuncID(uint32_t id) noexcept { _on_click_func_id = id; };
    inline uint32_t getOnClickFuncID() const noexcept { return _on_click_func_id; };
    // Function format for passive calls every frame.
    // - GLFWwindow* is the glfw window pointer
    // - uint32_t is the Plane ID
    using PassiveFunc = void(*)(std::shared_ptr<Window>, std::unique_ptr<T> const&);
    // Sets the function (referenced by ID) to be called every frame.
    inline void setPassiveFuncID(uint32_t id) noexcept { _passive_func_id = id; };
    inline uint32_t getPassiveFuncID() const noexcept { return _passive_func_id; };

    static std::map<uint32_t, OnClickFunc> on_click_funcs;
    static std::map<uint32_t, PassiveFunc> passive_funcs;

protected:
    // Model part of the MVP matrix, computed by scaling,
    // rotation, and translation of the original vertices.
    glm::mat4 _scaling;
    glm::mat4 _rotation;
    glm::mat4 _translation;
    glm::mat4 _model_mat4;
    bool _should_compute_mat4{ true };

private:
    // Function (referenced by ID) called when the Model is clicked.
    uint32_t _on_click_func_id{ 0 };
    // Function (referenced by ID) called every frame.
    uint32_t _passive_func_id{ 0 };
};

template <class T>
std::map<uint32_t, typename Model<T>::OnClickFunc> Model<T>::on_click_funcs {
    { 0, nullptr }
};

template <class T>
std::map<uint32_t, typename Model<T>::PassiveFunc> Model<T>::passive_funcs {
    { 0, nullptr }
};

template <class T>
Model<T>::Model(std::weak_ptr<Window> window, uint32_t id) try
    : _internal::WindowObjectWithID(window, id)
{
    setScaling();
    setRotation();
    setTranslation();
}
CATCH_AND_RETHROW_METHOD_EXC;

template <class T>
Model<T>::~Model()
{
}

template <class T>
void Model<T>::setScaling(glm::vec3 scaling)
{
    _scaling = glm::scale(glm::mat4(1), scaling);
    _should_compute_mat4 = true;
}

template <class T>
void Model<T>::setScaling(float scaling)
{
    setScaling(glm::vec3(scaling));
}

template <class T>
void Model<T>::scale(glm::vec3 scaling)
{
    _scaling = glm::scale(_scaling, scaling);
    _should_compute_mat4 = true;
}

template <class T>
void Model<T>::scale(float scaling)
{
    scale(glm::vec3(scaling));
}

template <class T>
void Model<T>::setRotation(glm::vec3 angles)
{
    _rotation = glm::rotate(glm::mat4(1), glm::radians(0.f), glm::vec3(0, 0, -1));
    rotate(angles);
}

template <class T>
void Model<T>::rotate(glm::vec3 angles)
{
    if (angles.x != 0.f)
        _rotation = glm::rotate(_rotation, glm::radians(angles.x), glm::vec3(1, 0, 0));
    if (angles.y != 0.f)
        _rotation = glm::rotate(_rotation, glm::radians(angles.y), glm::vec3(0, 1, 0));
    if (angles.z != 0.f)
        _rotation = glm::rotate(_rotation, glm::radians(angles.z), glm::vec3(0, 0, 1));
    _should_compute_mat4 = true;
}

template <class T>
void Model<T>::setTranslation(glm::vec3 position)
{
    _translation = glm::translate(glm::mat4(1), position);
    _should_compute_mat4 = true;
}

template <class T>
void Model<T>::translate(glm::vec3 translation)
{
    _translation = glm::translate(_translation, translation);
    _should_compute_mat4 = true;
}

template <class T>
glm::mat4 Model<T>::getModelMat4()
{
    if (_should_compute_mat4) {
        _model_mat4 = _translation * _rotation * _scaling;
        _should_compute_mat4 = false;
    }
    return _model_mat4;
}

template <class T>
void Model<T>::getAllTransformations(glm::vec3& scaling, glm::vec3& rot_angles, glm::vec3& translation)
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

SSS_GL_END;