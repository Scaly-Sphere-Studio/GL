#ifndef SSS_GL_MODEL_HPP
#define SSS_GL_MODEL_HPP

#include "SSS/GL/Window.hpp"
#include <glm/gtx/matrix_decompose.hpp>

/** @file
 *  Defines template abstract class SSS::GL::Model.
 */

SSS_GL_BEGIN;

/** Template abstract base class for %Model matrix.
 *  @sa Plane
 */
template <class T>
class Model : public _internal::SharedWindowObject<T> {
    friend class Window;

protected:
    /** Constructor, ensures the renderer is bound to a Window instance.*/
    Model(std::shared_ptr<Window> window);

public:
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

    /** Function format which is called every frame (from pollEverything()).
     *  Parameters are Window::Shared, and the derived %Model's shared_ptr
     */
    using PassiveFunc = void(*)(std::shared_ptr<Window>, std::shared_ptr<T>);
    /** Function format which is called on mouse-click events.
     *  Parameters are Window::Shared, the derived %Model's shared_ptr,
     *  and then follows glfw mouse-click event syntax.
     */
    using OnClickFunc = void(*)(std::shared_ptr<Window>,
        std::shared_ptr<T>, int, int, int);
    /** Map of user-defined #PassiveFunc (0 is reserved for nullptr).*/
    static std::map<uint32_t, PassiveFunc> passive_funcs;
    /** Map of user-defined #OnClickFunc (0 is reserved for nullptr).*/
    static std::map<uint32_t, OnClickFunc> on_click_funcs;
    /** Sets the #passive_funcs ID to be used for this instance.*/
    inline void setPassiveFuncID(uint32_t id) noexcept { _passive_func_id = id; };
    /** Sets the #on_click_funcs ID to be used for this instance.*/
    inline void setOnClickFuncID(uint32_t id) noexcept { _on_click_func_id = id; };
    /** Returns the #passive_funcs ID used for this instance.*/
    inline uint32_t getPassiveFuncID() const noexcept { return _passive_func_id; };
    /** Returns the #on_click_funcs ID used for this instance.*/
    inline uint32_t getOnClickFuncID() const noexcept { return _on_click_func_id; };

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

private:
    /** #passive_funcs ID used for this instance.*/
    uint32_t _passive_func_id{ 0 };
    /** #on_click_funcs ID used for this instance.*/
    uint32_t _on_click_func_id{ 0 };
};

template <class T>
std::map<uint32_t, typename Model<T>::PassiveFunc> Model<T>::passive_funcs {};

template <class T>
std::map<uint32_t, typename Model<T>::OnClickFunc> Model<T>::on_click_funcs {};

template <class T>
Model<T>::Model(std::shared_ptr<Window> window) try
    : _internal::SharedWindowObject<T>(window)
{
    setScaling();
    setRotation();
    setTranslation();
}
CATCH_AND_RETHROW_METHOD_EXC;

template <class T>
Model<T>::~Model() = default;

template <class T>
void Model<T>::setScaling(glm::vec3 scaling)
{
    _scaling = glm::scale(glm::mat4(1), scaling);
    _should_compute_mat4 = true;
}

template <class T>
void Model<T>::scale(glm::vec3 scaling)
{
    _scaling = glm::scale(_scaling, scaling);
    _should_compute_mat4 = true;
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

template<class T>
glm::vec3 Model<T>::getScaling()
{
    glm::vec3 scaling, rot_angles, translation;
    getAllTransformations(scaling, rot_angles, translation);
    return scaling;
}

template<class T>
glm::vec3 Model<T>::getRotation()
{
    glm::vec3 scaling, rot_angles, translation;
    getAllTransformations(scaling, rot_angles, translation);
    return rot_angles;
}

template<class T>
glm::vec3 Model<T>::getTranslation()
{
    glm::vec3 scaling, rot_angles, translation;
    getAllTransformations(scaling, rot_angles, translation);
    return translation;
}

SSS_GL_END;

#endif // SSS_GL_MODEL_HPP