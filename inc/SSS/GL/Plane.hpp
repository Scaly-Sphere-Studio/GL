#pragma once

#include "_internal/callbacks.hpp"
#include "Model.hpp"
#include "Texture2D.hpp"

__SSS_GL_BEGIN

class Plane : public Model {
    friend class Context;
    friend class Texture2D;

protected:
    Plane(std::shared_ptr<Context> context);

public:
    virtual ~Plane();

    using Ptr = std::unique_ptr<Plane>;
    
    void useTexture(uint32_t texture_id, TextureType texture_type);

    virtual glm::mat4 getModelMat4() noexcept;
    void draw() const;

protected:
    uint32_t _texture_id{ 0 };
    TextureType _texture_type{ TextureType::None };
    GLsizei _tex_w{ 0 }, _tex_h{ 0 };
    glm::vec3 _tex_scaling{ 1 };
    
    void _updateTexScaling();
};

__SSS_GL_END