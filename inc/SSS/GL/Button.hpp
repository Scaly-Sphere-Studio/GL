#pragma once

#include "Plane.hpp"

__SSS_GL_BEGIN

class Button : public Plane {
    friend void _internal::mouse_button_callback(GLFWwindow* ptr, int button, int action, int mods);
    friend class Window;

protected:
    Button();
    Button(std::string const& filepath);

public:
    virtual ~Button() = default;

    using Shared = std::shared_ptr<Button>;

private:
    bool _WasItPressed(double x, double y);
};

__SSS_GL_END