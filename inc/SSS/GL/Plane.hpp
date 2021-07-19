#pragma once

#include "_internal/callbacks.hpp"
#include "Model.hpp"
#include "Texture2D.hpp"

__SSS_GL_BEGIN

class Plane : public Model {
    friend class Window;
    friend class Texture2D;

private:
    void _init_statics(std::shared_ptr<Window> window);

protected:
    Plane(std::shared_ptr<Window> window);
    Plane(std::shared_ptr<Window> window, TextureBase::Shared texture);
    using Weak = std::weak_ptr<Plane>;

private:
    static std::vector<Weak> _instances;

public:
    virtual ~Plane();

    using Ptr = std::unique_ptr<Plane>;
    using Shared = std::shared_ptr<Plane>;
    static Shared create(std::shared_ptr<Window> window);
    static Shared create(std::shared_ptr<Window> window, TextureBase::Shared texture);
    
    void useTexture(TextureBase::Shared texture);

    virtual glm::mat4 getModelMat4() noexcept;
    void draw() const;

protected:
    TextureBase::Shared _texture;
    GLsizei _tex_w{ 0 }, _tex_h{ 0 };
    glm::vec3 _tex_scaling{ 1 };
    
    void _updateTexScaling();
};

__SSS_GL_END