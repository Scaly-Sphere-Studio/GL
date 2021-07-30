#pragma once

#include "_internal/callbacks.hpp"
#include "Model.hpp"
#include "Texture.hpp"

__SSS_GL_BEGIN

class Plane : public Model {
    friend class Window;
    friend class Texture;

protected:
    Plane(std::weak_ptr<Window> window);

public:
    virtual ~Plane();

    using Ptr = std::unique_ptr<Plane>;
    
    void useTexture(uint32_t texture_id);

    virtual glm::mat4 getModelMat4() noexcept;
    void draw() const;

protected:
    uint32_t _texture_id{ 0 };
    bool _use_texture{ false };
    GLsizei _tex_w{ 0 }, _tex_h{ 0 };
    glm::vec3 _tex_scaling{ 1 };
    
    void _updateTexScaling();
};

__SSS_GL_END