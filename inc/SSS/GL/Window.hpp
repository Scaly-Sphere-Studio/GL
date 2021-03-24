#pragma once

#include "_includes.hpp"
#include "_pointers.hpp"
#include "_callbacks.hpp"
#include "Model.hpp"
#include "Plane.hpp"
#include "Button.hpp"

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
    friend void _internal::mouse_position_callback(GLFWwindow* ptr, double x, double y);
    friend void _internal::mouse_button_callback(GLFWwindow* ptr, int button, int action, int mods);
    friend void _internal::monitor_callback(GLFWmonitor* ptr, int event);
    friend void _internal::key_callback(GLFWwindow* ptr, int key, int scancode, int action, int mods);

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
    // Returns last focused window
    static Shared getMain();

// --- Model methods ---

    Model::Shared createModel();
    Plane::Shared createPlane();
    Plane::Shared createPlane(std::string filepath);
    Button::Shared createButton();
    Button::Shared createButton(std::string filepath);

    void unloadModel(Model::Shared model);
    void unloadPlane(Plane::Shared plane);
    void unloadButton(Button::Shared button);

    void unloadAllModels();
    void unloadAllPlanes();
    void unloadAllButtons();

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
        if (typeid(set) == typeid(glfwSetWindowSizeCallback)) {
            _resize_callback = GLFWwindowsizefun(callback);
        }
        else if (typeid(set) == typeid(glfwSetWindowPosCallback)) {
            _pos_callback = GLFWwindowposfun(callback);
        }
        else if (typeid(set) == typeid(glfwSetKeyCallback)) {
            _key_callback = GLFWkeyfun(callback);
        }
        else if (typeid(set) == typeid(glfwSetCursorPosCallback)) {
            _mouse_position_callback = GLFWcursorposfun(callback);
        }
        else if (typeid(set) == typeid(glfwSetMouseButtonCallback)) {
            _mouse_button_callback = GLFWmousebuttonfun(callback);
        }
        else {
            set(_window.get(), callback);
        }
    };

    using KeyInputs = std::array<bool, GLFW_KEY_LAST + 1>;
    inline KeyInputs const& getKeyInputs() const noexcept { return _key_inputs; }

    // Returns the monitor on which the windowed is considered to be.
    // -> See internal callback window_pos_callback();
    inline GLFWmonitor* getMonitor() const noexcept { return _main_monitor.ptr; }

    inline float getScreenRatio() const noexcept
        { return static_cast<float>(_w) / static_cast<float>(_h); }

private:
// --- Private variables ---

    std::vector<Model::Shared> _models;
    std::vector<Plane::Shared> _planes;
    std::vector<Button::Shared> _buttons;

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

    GLFWwindowsizefun   _resize_callback{ nullptr };
    GLFWwindowposfun    _pos_callback{ nullptr };
    GLFWkeyfun          _key_callback{ nullptr };
    GLFWcursorposfun    _mouse_position_callback{ nullptr };
    GLFWmousebuttonfun  _mouse_button_callback{ nullptr };

    KeyInputs _key_inputs;

    // FPS Timer
    FPS_Timer fps_timer;

    void _setMainMonitor(_internal::Monitor const& monitor);
};

__SSS_GL_END