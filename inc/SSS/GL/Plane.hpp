#pragma once

#include "_callbacks.hpp"
#include "Model.hpp"
#include "Texture2D.hpp"

__SSS_GL_BEGIN

class Plane : public Model {
    friend class Window;

private:
    static VAO::Shared _static_vao;
    static VBO::Shared _static_vbo;
    static IBO::Shared _static_ibo;
    static void _init_statics();

protected:
    Plane();
    Plane(Texture2D::Shared texture);
public:
    virtual ~Plane() = default;

    using Shared = std::shared_ptr<Plane>;
    
    void useTexture(Texture2D::Shared texture);

    virtual glm::mat4 getModelMat4() noexcept;
    void draw() const;

protected:
    Texture2D::Shared _texture;
    GLsizei _tex_w{ 0 }, _tex_h{ 0 };
    glm::vec3 _tex_scaling{ 1 };
    
    void _updateTexScaling();
};

__SSS_GL_END