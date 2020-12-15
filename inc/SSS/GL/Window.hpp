#pragma once

#include "SSS/GL/_includes.hpp"
#include "SSS/GL/_pointers.hpp"
#include "SSS/GL/_callbacks.hpp"

__SSS_GL_BEGIN

__INTERNAL_BEGIN
struct Monitor {
    GLFWmonitor* ptr;   // GLFW pointer
    float w;    // Width, in inches
    float h;    // Height, in inches
};
__INTERNAL_END

    // --- Window class ---
    
class Window {
    
    friend void _internal::window_resize_callback(GLFWwindow* ptr, int w, int h);
    friend void _internal::window_pos_callback(GLFWwindow* ptr, int x, int y);
    friend void _internal::monitor_callback(GLFWmonitor* ptr, int event);

public:
// --- Log options ---
    
    struct LOG {
        static bool constructor;
        static bool destructor;
        static bool fps;
        static bool dpi_update;
    };

// --- Public aliases ---

    using Shared = std::shared_ptr<Window>;

private:
// --- Instances storage (private) ---

    using Weak = std::weak_ptr<Window>;

    // Instances of created windows
    static std::map<GLFWwindow const*, Weak> _instances;
    // All connected monitors
    static std::vector<_internal::Monitor> _monitors;

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

    // Renders a frame & polls events.
    // Logs fps if specified in LOG structure.
    void render();

    // Wether the user requested to close the window.
    // NOTE: this simply is a call to glfwWindowShouldClose
    bool shouldClose() const noexcept;

    // Enables or disables the VSYNC of the window
    void setVSYNC(bool state);
    // Enables or disables fullscreen mode on given screen
    void setFullscreen(bool state, int screen_id = -1);
    // Sets a corresponding callback
    template<typename _Func>
    void setCallback(_Func(*set)(GLFWwindow*, _Func), _Func callback) {
        set(_window.get(), callback);
    };

    // Returns the monitor on which the windowed is considered to be.
    // -> See internal callback window_pos_callback();
    inline GLFWmonitor* getMonitor() const noexcept { return _main_monitor.ptr; }

private:
// --- Private variables ---

    // Window size
    int _w; // Width
    int _h; // Height

    // Windowed to Fullscreen variables
    int _windowed_x{ 0 };   // Old x (left) pos
    int _windowed_y{ 0 };   // Old y (up) pos
    
    // Window ptr. Automatically destroyed
    _internal::GLFWwindow_Ptr _window;

    // Main monitor the window is on
    _internal::Monitor _main_monitor;

    // FPS Timer
    FPS_Timer fps_timer;

    void _setMainMonitor(_internal::Monitor const& monitor);
};

__SSS_GL_END