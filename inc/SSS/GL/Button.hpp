#pragma once

#include "Plane.hpp"

__SSS_GL_BEGIN

class Button : public Plane {
    friend void _internal::mouse_position_callback(GLFWwindow* ptr, double x, double y);
    friend void _internal::mouse_button_callback(GLFWwindow* ptr, int button, int action, int mods);
    friend class Window;

protected:
    Button();
    Button(std::string const& filepath);

public:
    virtual ~Button() = default;

    using Shared = std::shared_ptr<Button>;

    inline bool IsHovered() const noexcept { return _is_hovered; };

private:
    bool _is_hovered{ false };
    void _updateHoverStatus(double x, double y);
};

__SSS_GL_END