#pragma once

#include "SSS/GLFW/_includes.hpp"
#include "SSS/GLFW/_pointers.hpp"

__SSS_GLFW_BEGIN

    // --- Callbacks ---

__INTERNAL_BEGIN
// Resizes the internal width and height of correspondig Window instance
void window_resize_callback(GLFWwindow* ptr, int w, int h);
__INTERNAL_END

    // --- Window class ---
    
class Window {
    
    friend void _internal::window_resize_callback(GLFWwindow* ptr, int w, int h);

public:
// --- Log options ---
    
    struct LOG {
        static bool constructor;
        static bool destructor;
        static bool fps;
    };

// --- Public aliases ---

    using Shared = std::shared_ptr<Window>;

private:
// --- Instances storage (private) ---

    using Weak = std::weak_ptr<Window>;

    // Instances of created windows
    static std::map<GLFWwindow const*, Weak> _instances;

    // Constructor, creates a window and makes its context current
    // Private, to be called via Window::create();
    Window(int w, int h, std::string const& title);

public :
// --- Instances storage (public) ---

    // Destructor
    ~Window();

    // Creates a window and returns a corresponding shared_ptr
    static Shared create(int w, int h, std::string const& title = "Untitled");
    // To be used in callbacks.
    // Returns an existing Window instance, via its GLFWwindow pointer
    static Shared get(GLFWwindow const* ptr);

// --- Public methods ---

    // Wether the user requested to close the window.
    // NOTE: this simply is a call to glfwWindowShouldClose
    bool shouldClose() const noexcept;

    // Enables or disables the VSYNC of the window
    void setVSYNC(bool state);
    // Enables or disables fullscreen mode on given screen
    void setFullscreen(bool state, int screen_id = 0);

    // Renders a frame & polls events.
    // Logs fps if specified in LOG structure.
    void render();

    // Sets a corresponding callback
    template<typename _Func>
    void setCallback(_Func(*set)(GLFWwindow*, _Func), _Func callback) {
        set(_window.get(), callback);
    };

private:
// --- Private variables ---

    // Window size
    int _w; // Width
    int _h; // Height

    // Windowed to Fullscreen variables
    int _windowed_x{ 0 };   // Old x (left) pos
    int _windowed_y{ 0 };   // Old y (up) pos
    int _windowed_w{ 0 };   // Old width
    int _windowed_h{ 0 };   // Old height
    
    // Window ptr. Automatically destroyed
    _internal::GLFWwindow_Ptr _window;

    // FPS Timer
    FPS_Timer fps_timer;
};

__SSS_GLFW_END