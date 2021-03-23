#pragma once

#include "_callbacks.hpp"
#include "Model.hpp"

__SSS_GL_BEGIN

class Plane : public Model {
    friend void _internal::window_resize_callback(GLFWwindow* ptr, int w, int h);
    friend class Window;

private:
    static VAO::Shared _static_vao;
    static VBO::Shared _static_vbo;
    static IBO::Shared _static_ibo;
    static void _init_statics();

protected:
    Plane();
    Plane(std::string const& filepath);

public:
    virtual ~Plane() = default;

    using Shared = std::shared_ptr<Plane>;
    // Creates a Plane model and returns a shared_ptr
    static Shared create();
    // Creates a Plane model and returns a shared_ptr
    static Shared create(std::string const& filepath);

    static void unload(Shared instance);
    static void unloadAll();
    
    void editTexture(const GLvoid* pixels, GLsizei width, GLsizei height,
        GLenum format = GL_RGBA, GLint internalformat = GL_RGBA,
        GLenum type = GL_UNSIGNED_BYTE, GLint level = 0);

    virtual glm::mat4 getModelMat4() noexcept;
    void draw() const;

protected:
    Texture _texture;
    GLsizei _tex_w{ 0 }, _tex_h{ 0 };
    glm::vec3 _tex_scaling{ 1 };
    glm::vec3 _win_scaling{ 1 };
    
    void _updateTexScaling(int width, int height);
    void _updateWinScaling();
};

__SSS_GL_END