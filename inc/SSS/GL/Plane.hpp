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
    void _renderPart(Mat4_array const& Models, uint32_t& count, bool reset_depth) const;

public:
    virtual void render() const;

private:
    bool _findNearestModel(float x, float y);
    uint32_t _hovered_plane{ 0 };
    double _hovered_z{ 0.f };
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

    inline void useAsButton(bool state) noexcept { _use_as_button = state; };
    inline bool isButton() const noexcept { return _use_as_button; };
    
    // Format to be used in setFunction();
    using ButtonFunction = void(*)();
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
    // Mouse hovering relative position, updated via Window::render every x ms.
    int _relative_x{ 0 };
    int _relative_y{ 0 };

    // Called from _isHovered
    bool _hoverTriangle(glm::mat4 const& mvp, glm::vec4 const& A,
        glm::vec4 const& B, glm::vec4 const& C, float x, float y,
        double &z, bool& is_hovered);
    // Returns true and updates z if Plane is hovered
    bool _isHovered(Camera::Ptr const& camera, float x, float y, double &z);
};

__SSS_GL_END