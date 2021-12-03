#pragma once

#include "_internal/pointers.hpp"
#include "Shaders.hpp"

__SSS_GL_BEGIN

enum class ModelType {
    Classic,    // Model class
    Plane,      // Plane class
};

class Model : public _internal::WindowObject {
    friend class Window;

protected:
    Model(std::weak_ptr<Window> window);

public:
    virtual ~Model();

    using Ptr = std::unique_ptr<Model>;

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
    using OnClickFunc = void(*)(GLFWwindow*, uint32_t, int, int, int);
    // Sets the function (referenced by ID) to be called when the Plane is clicked.
    inline void setOnClickFuncID(uint32_t id) noexcept { _on_click_func_id = id; };
    inline uint32_t getOnClickFuncID() const noexcept { return _on_click_func_id; };
    // Function format for passive calls every frame.
    // - GLFWwindow* is the glfw window pointer
    // - uint32_t is the Plane ID
    using PassiveFunc = void(*)(GLFWwindow*, uint32_t);
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

    // Function (referenced by ID) called when the Model is clicked.
    uint32_t _on_click_func_id{ 0 };
    // Called whenever the button is clicked (determined by Hitbox).
    void _callOnClickFunction(GLFWwindow* ptr, uint32_t id, int button, int action, int mods);
    // Function (referenced by ID) called every frame.
    uint32_t _passive_func_id{ 0 };
    // Called every frame.
    void _callPassiveFunction(GLFWwindow* ptr, uint32_t id);
};

__SSS_GL_END

