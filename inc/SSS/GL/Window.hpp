#pragma once

#include "_internal/pointers.hpp"
#include "_internal/callbacks.hpp"
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
    
class Window : public _internal::ContextHolder {
    
    friend void _internal::window_resize_callback(GLFWwindow* ptr, int w, int h);
    friend void _internal::window_pos_callback(GLFWwindow* ptr, int x, int y);
    friend void _internal::mouse_position_callback(GLFWwindow* ptr, double x, double y);
    friend void _internal::mouse_button_callback(GLFWwindow* ptr, int button, int action, int mods);
    friend void _internal::key_callback(GLFWwindow* ptr, int key, int scancode, int action, int mods);
    friend void _internal::monitor_callback(GLFWmonitor* ptr, int event);

    friend class Context;
    friend class ContextLocker;

public:
// --- Log options ---
    
    struct LOG {
        static bool constructor;
        static bool destructor;
        static bool fps;
    };

    struct Args {
        int w{ 720 };
        int h{ 720 };
        std::string title{ "Untitled" };
        int monitor_id{ 0 };
        bool fullscreen{ false };
    };

// --- Public aliases ---

    using Ptr = std::unique_ptr<Window>;

private:
    // All connected monitors
    static std::vector<_internal::Monitor> _monitors;

    // Constructor, creates a window and makes its context current
    // Private, to be called via Window::create();
    Window(GLFWwindow* win_ptr, Args const& args);

public :
    // Rule of 5
    ~Window();                                      // Destructor
    Window(const Window&)               = delete;   // Copy constructor
    Window(Window&&)                    = delete;   // Move constructor
    Window& operator=(const Window&)    = delete;   // Copy assignment
    Window& operator=(Window&&)         = delete;   // Move assignment

    struct Objects {
        std::map<uint32_t, VAO::Ptr> VAOs;
        std::map<uint32_t, Program::Ptr> programs;
    };

private:
    Objects _objects;

public:
    inline Objects const& getObjects() const noexcept { return _objects; };
    void cleanObjects() noexcept;

    void createVAO(uint32_t id);
    void removeVAO(uint32_t id);

    void createShaders(uint32_t id, std::string const& vertex_fp, std::string const& fragment_fp);
    void removeShaders(uint32_t id);

    void createRenderer();
    void removeRenderer();

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
    void setCallback(_Func(*set)(GLFWwindow*, _Func), _Func callback)
    {
        void const* ptr(set);
        if (ptr == (void*)(&glfwSetWindowSizeCallback)) {
            _resize_callback = GLFWwindowsizefun(callback);
        }
        else if (ptr == (void*)(&glfwSetWindowPosCallback)) {
            _pos_callback = GLFWwindowposfun(callback);
        }
        else if (ptr == (void*)(&glfwSetKeyCallback)) {
            _key_callback = GLFWkeyfun(callback);
        }
        else if (ptr == (void*)(&glfwSetCursorPosCallback)) {
            _mouse_position_callback = GLFWcursorposfun(callback);
        }
        else if (ptr == (void*)(&glfwSetMouseButtonCallback)) {
            _mouse_button_callback = GLFWmousebuttonfun(callback);
        }
        else {
            set(_window.get(), callback);
        }
    };

    void setFOV(float radians);
    void setProjectionRange(float z_near, float z_far);

    using KeyInputs = std::array<bool, GLFW_KEY_LAST + 1>;
    inline KeyInputs const& getKeyInputs() const noexcept { return _key_inputs; }

    // Returns raw GLFWwindow ptr.
    inline GLFWwindow* getGLFWwindow() const { return _window.get(); };
    // Returns the monitor on which the windowed is considered to be.
    // -> See internal callback window_pos_callback();
    inline GLFWmonitor* getMonitor() const noexcept { return _main_monitor.ptr; }

    inline glm::mat4 getPerspective() const noexcept { return _perspe_mat4; }
    inline glm::mat4 getOrtho() const noexcept { return _ortho_mat4; }

    inline float getScreenRatio() const noexcept
        { return static_cast<float>(_w) / static_cast<float>(_h); }

private:
// --- Private variables ---

    // Window size
    int _w; // Width
    int _h; // Height
    // Windowed to Fullscreen variables
    int _windowed_x{ 0 };   // Old x (left) pos
    int _windowed_y{ 0 };   // Old y (up) pos
    
    // Main monitor the window is on
    _internal::Monitor _main_monitor;

    float _fov{ glm::radians(70.f) };
    float _z_near{ 0.1f }, _z_far{ 100.f };
    // Perspective projection, set by window dimensions and user given FOV
    glm::mat4 _perspe_mat4;
    // Ortho projection, set by window dimensions
    glm::mat4 _ortho_mat4;

    // Internal callbacks that are called before calling within themselves user callbacks
    GLFWwindowsizefun   _resize_callback{ nullptr };            // Window resize
    GLFWwindowposfun    _pos_callback{ nullptr };               // Window position
    GLFWkeyfun          _key_callback{ nullptr };               // Window keyboard key
    GLFWcursorposfun    _mouse_position_callback{ nullptr };    // Window mouse position
    GLFWmousebuttonfun  _mouse_button_callback{ nullptr };      // Window mouse button

    // Array of keyboard keys being currently pressed
    KeyInputs _key_inputs;

    // FPS Timer
    FPS_Timer _fps_timer;

    // Sets the window's main monitor
    void _setMainMonitor(_internal::Monitor const& monitor);

    // Calculates both projetions based on internal variables
    void _setProjections();
};

__SSS_GL_END