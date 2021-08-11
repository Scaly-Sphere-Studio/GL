#pragma once

#include "_internal/callbacks.hpp"
#include "Model.hpp"
#include "Texture.hpp"
#include "Shaders.hpp"
#include "Renderer.hpp"

__SSS_GL_BEGIN

class PlaneRenderer : public Renderer {
    friend class Window;
protected:
    PlaneRenderer(std::weak_ptr<Window> window);
public:
    virtual void render() const;
};

class Plane : public Model {
    friend void _internal::mouse_position_callback(GLFWwindow* ptr, double x, double y);
    friend class Window;
    friend class Texture;
    friend class PlaneRenderer;

protected:
    Plane(std::weak_ptr<Window> window);

public:
    virtual ~Plane();

    using Ptr = std::unique_ptr<Plane>;
    using Renderer = PlaneRenderer;
    
    void useTexture(uint32_t texture_id);

    virtual glm::mat4 getModelMat4();

    // Format to be used in setFunction();
    using ButtonFunction = void(*)();

    inline void useAsButton(bool state) noexcept { _use_as_button = state; };

    // Returns if the button is currently hovered by the mouse.
    inline bool isHovered() const noexcept { return _is_hovered; };

    // Sets the function to be called when the button is clicked.
    // The function MUST be of the format void (*)();
    void setFunction(ButtonFunction func);
    // Calls the function set via setFunction();
    // Called whenever the button is clicked.
    void callFunction();

protected:
    uint32_t _texture_id{ 0 };
    bool _use_texture{ false };
    GLsizei _tex_w{ 0 }, _tex_h{ 0 };
    glm::vec3 _tex_scaling{ 1 };
    
    void _updateTexScaling();

    // Whether the Plane should be treated as a button
    bool _use_as_button{ false };
    // Function called when the button is clicked.
    ButtonFunction _f{ nullptr };
    // Mouse hovering state, always updated via the mouse position callback.
    bool _is_hovered{ false };
    int _relative_x{ 0 };
    int _relative_y{ 0 };

    // Called from _updateHoverStatus
    bool _hoverTriangle(glm::vec3 const& A, glm::vec3 const& B,
        glm::vec3 const& C, glm::vec3 const& P, bool is_abc);
    // Updates _is_hovered via the mouse position callback.
    void _updateHoverStatus(double x, double y);
};

__SSS_GL_END