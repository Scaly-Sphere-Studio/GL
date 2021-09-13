#pragma once

#include "_internal/callbacks.hpp"
#include "Model.hpp"
#include "Texture.hpp"
#include "Camera.hpp"
#include "Renderer.hpp"

__SSS_GL_BEGIN

class PlaneRenderer : public Renderer {
    friend class Window;
protected:
    static constexpr uint32_t glsl_max_array_size = 128;

    PlaneRenderer(std::weak_ptr<Window> window);

    using Mat4_array = std::array<glm::mat4, glsl_max_array_size>;
    void _renderPart(uint32_t& count, bool reset_depth) const;

public:
    virtual void render();

private:
    bool _findNearestModel(float x, float y);
    uint32_t _hovered_plane{ 0 };
    double _hovered_z{ DBL_MAX };
    Mat4_array _VPs;
    Mat4_array _Models;
};

class Plane : public Model {
    friend void _internal::mouse_button_callback(GLFWwindow* ptr, int button, int action, int mods);
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

    enum class Hitbox {
        None,   // No hitbox.
        Alpha,  // Hitbox if alpha > 0.
        Full    // Hitbox if within plane coordinates.
    };
    inline void setHitbox(Hitbox hitbox) noexcept { _hitbox = hitbox; };
    inline Hitbox getHitbox() const noexcept { return _hitbox; };
    
    // Format to be used in setFunction();
    using ButtonFunction = void(*)(GLFWwindow*, uint32_t, int, int, int);
    // Sets the function to be called when the button is clicked.
    // The function MUST be of the format void (*)();
    void setFunction(ButtonFunction func);

protected:
    uint32_t _texture_id{ 0 };
    bool _use_texture{ false };
    GLsizei _tex_w{ 0 }, _tex_h{ 0 };
    glm::vec3 _tex_scaling{ 1 };
    
    void _updateTexScaling();

    // Type of hitbox
    Hitbox _hitbox{ Hitbox::None };
    // Function called when the button is clicked.
    ButtonFunction _f{ nullptr };
    // Mouse hovering relative position, updated via Window::render every x ms.
    int _relative_x{ 0 };
    int _relative_y{ 0 };
    // Calls the function set via setFunction();
    // Called whenever the button is clicked.
    void _callFunction(GLFWwindow* ptr, uint32_t id, int button, int action, int mods);

    // Called from _isHovered
    bool _hoverTriangle(glm::mat4 const& mvp, glm::vec4 const& A,
        glm::vec4 const& B, glm::vec4 const& C, float x, float y,
        double &z, bool& is_hovered);
    // Returns true and updates z if Plane is hovered
    bool _isHovered(glm::mat4 const& VP, float x, float y, double &z);
};

__SSS_GL_END