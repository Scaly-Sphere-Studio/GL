#pragma once

#include "Plane.hpp"

__SSS_GL_BEGIN

class Button : public Plane {
    friend void _internal::mouse_position_callback(GLFWwindow* ptr, double x, double y);
    friend void _internal::window_resize_callback(GLFWwindow* ptr, int w, int h);
    friend class Window;

protected:
    Button() = default;
    Button(Texture2D::Shared texture, GLFWwindow const* context);

public:
    virtual ~Button() = default;

    // Shared pointer
    using Shared = std::shared_ptr<Button>;
    // Format to be used in setFunction();
    using ButtonFunction = void(*)();

    // Returns if the button is currently hovered by the mouse.
    inline bool isHovered() const noexcept { return _is_hovered; };

    // Sets the function to be called when the button is clicked.
    // The function MUST be of the format void (*)();
    void setFunction(ButtonFunction func);
    // Calls the function set via setFunction();
    // Called whenever the button is clicked.
    void callFunction();

    inline glm::mat4 getMVP() noexcept { return getModelMat4(); };

private:
    virtual glm::mat4 getModelMat4() noexcept;

    glm::vec3 _win_scaling{ 1 };
    // Function called when the button is clicked.
    ButtonFunction _f{ nullptr };
    // Mouse hovering state, always updated via the mouse position callback.
    bool _is_hovered{ false };

    // Updates window scaling when window is resized.
    void _updateWinScaling(GLFWwindow const* context);
    // Updates _is_hovered via the mouse position callback.
    void _updateHoverStatus(double x, double y);
};

__SSS_GL_END