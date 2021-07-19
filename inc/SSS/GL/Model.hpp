#pragma once

#include "_internal/pointers.hpp"
#include "Shaders.hpp"

__SSS_GL_BEGIN

enum class Transformation {
    None        = 0,
    Scaling     = 1 << 0,
    Rotation    = 1 << 1,
    Translation = 1 << 2,
    All         = Scaling | Rotation | Translation,
};
__ENABLE_BITMASK_OPERATORS(Transformation);

class Model : public _internal::WindowObject {
    friend class Window;

protected:
    Model(std::shared_ptr<Window> window);
    using Weak = std::weak_ptr<Model>;

private:
    static std::vector<Weak> _instances;

public:
    virtual ~Model();

    using Ptr = std::unique_ptr<Model>;
    using Shared = std::shared_ptr<Model>;
    static Shared create(std::shared_ptr<Window> window);

    void scale(glm::vec3 scaling);
    void scale(float scaling);
    void rotate(float radians, glm::vec3 axis);
    void translate(glm::vec3 translation);

    void resetTransformations(Transformation transformations);

    virtual glm::mat4 getModelMat4() noexcept;

protected:

    VAO::Shared _vao;
    VBO::Shared _vbo;
    IBO::Shared _ibo;

    glm::mat4 _scaling;
    glm::mat4 _rotation;
    glm::mat4 _translation;

    glm::mat4 _model_mat4;
    bool _should_compute_mat4{ true };
};

__SSS_GL_END

