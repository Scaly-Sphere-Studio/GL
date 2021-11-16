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

protected:
    // Model part of the MVP matrix, computed by scaling,
    // rotation, and translation of the original vertices.
    glm::mat4 _scaling;
    glm::mat4 _rotation;
    glm::mat4 _translation;
    glm::mat4 _model_mat4;
    bool _should_compute_mat4{ true };
};

__SSS_GL_END

