#pragma once

#include "_internal/pointers.hpp"
#include "_internal/callbacks.hpp"
#include "Model.hpp"
#include "Plane.hpp"
#include "Camera.hpp"
#include "Shaders.hpp"
#include "Renderer.hpp"

__SSS_GL_BEGIN

__INTERNAL_BEGIN
struct Monitor {
    GLFWmonitor* ptr;   // GLFW pointer
    float w;    // Width, in inches
    float h;    // Height, in inches
};
__INTERNAL_END

    // --- Window class ---
    
class Window : public std::enable_shared_from_this<Window> {
    
    friend void _internal::window_iconify_callback(GLFWwindow* ptr, int state);
    friend void _internal::window_resize_callback(GLFWwindow* ptr, int w, int h);
    friend void _internal::window_pos_callback(GLFWwindow* ptr, int x, int y);
    friend void _internal::mouse_position_callback(GLFWwindow* ptr, double x, double y);
    friend void _internal::mouse_button_callback(GLFWwindow* ptr, int button, int action, int mods);
    friend void _internal::key_callback(GLFWwindow* ptr, int key, int scancode, int action, int mods);
    friend void _internal::monitor_callback(GLFWmonitor* ptr, int event);

    friend class Context;

public:
// --- Log options ---
    
    struct LOG {
        static bool constructor;
        static bool destructor;
        static bool glfw_init;
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

    using Shared = std::shared_ptr<Window>;

private:
    using Weak = std::weak_ptr<Window>;
    // All Window instances
    static std::vector<Weak> _instances;
    // All connected monitors
    static std::vector<_internal::Monitor> _monitors;

    // Constructor, creates a window
    // Private, to be called via Window::create();
    Window(Args const& args);

public :
    // Rule of 5
    ~Window();                                      // Destructor
    Window(const Window&)               = delete;   // Copy constructor
    Window(Window&&)                    = delete;   // Move constructor
    Window& operator=(const Window&)    = delete;   // Copy assignment
    Window& operator=(Window&&)         = delete;   // Move assignment

    static Shared create(Args const& args);
    static Shared get(GLFWwindow* ptr);

    // All context bound objects
    struct Objects {
        // Rule of 5
        Objects()                           = default;  // Constructor
        ~Objects()                          = default;  // Destructor
        Objects(const Objects&)             = delete;   // Copy constructor
        Objects(Objects&&)                  = delete;   // Move constructor
        Objects& operator=(const Objects&)  = delete;   // Copy assignment
        Objects& operator=(Objects&&)       = delete;   // Move assignment
        // Objects
        std::map<uint32_t, Model::Ptr> models;          // Models
        std::map<uint32_t, Plane::Ptr> planes;          // Planes
        std::map<uint32_t, Texture::Ptr> textures;      // Textures
        std::map<uint32_t, Camera::Ptr> cameras;        // Cameras
        std::map<uint32_t, Shaders::Ptr> shaders;       // Shaders
        std::map<uint32_t, Renderer::Ptr> renderers;    // Renderers
    };

private:
    Objects _objects;

public:
    inline Objects const& getObjects() const noexcept { return _objects; };
    void cleanObjects() noexcept;

    void createModel(uint32_t id, ModelType type);
    void removeModel(uint32_t id, ModelType type);

    void createTexture(uint32_t id);
    void removeTexture(uint32_t id);
    static void pollTextureThreads();

    void createCamera(uint32_t id);
    void removeCamera(uint32_t id);

    void createShaders(uint32_t id);
    void removeShaders(uint32_t id);

    template<class T = Renderer>
    void createRenderer(uint32_t id) {
        _objects.renderers.try_emplace(id);
        _objects.renderers.at(id).reset(new T(weak_from_this()));
    }
    void removeRenderer(uint32_t id);

// --- Public methods ---

    // Renders a frame & polls events.
    // Logs fps if specified in LOG structure.
    void render();
private:
    std::chrono::steady_clock::time_point _last_render_time;
    std::chrono::steady_clock::duration _hover_waiting_time;
    void _updateHoveredButton(std::chrono::steady_clock::time_point const& now);

public:

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
        if (ptr == (void*)(&glfwSetWindowIconifyCallback)) {
            _iconify_callback = GLFWwindowiconifyfun(callback);
        }
        else if (ptr == (void*)(&glfwSetWindowSizeCallback)) {
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

    using KeyInputs = std::array<bool, GLFW_KEY_LAST + 1>;
    inline KeyInputs const& getKeyInputs() const noexcept { return _key_inputs; }

    // Returns raw GLFWwindow ptr.
    inline GLFWwindow* getGLFWwindow() const { return _window.get(); };
    // Returns the monitor on which the windowed is considered to be.
    // -> See internal callback window_pos_callback();
    inline GLFWmonitor* getMonitor() const noexcept { return _main_monitor.ptr; }

    inline float getScreenRatio() const noexcept
        { return static_cast<float>(_w) / static_cast<float>(_h); }
    
    inline bool isIconified() const noexcept { return _is_iconified; };

private:
// --- Private variables ---

    // Window size
    int _w; // Width
    int _h; // Height
    // Windowed to Fullscreen variables
    int _windowed_x{ 0 };   // Old x (left) pos
    int _windowed_y{ 0 };   // Old y (up) pos
    // Iconify state
    bool _is_iconified{ false };

    // GLFWwindow ptr
    _internal::GLFWwindow_Ptr _window;    
    // Main monitor the window is on
    _internal::Monitor _main_monitor;

    // Internal callbacks that are called before calling within themselves user callbacks
    GLFWwindowiconifyfun _iconify_callback{ nullptr };          // Window iconify
    GLFWwindowsizefun    _resize_callback{ nullptr };           // Window resize
    GLFWwindowposfun     _pos_callback{ nullptr };              // Window position
    GLFWkeyfun           _key_callback{ nullptr };              // Window keyboard key
    GLFWcursorposfun     _mouse_position_callback{ nullptr };   // Window mouse position
    GLFWmousebuttonfun   _mouse_button_callback{ nullptr };     // Window mouse button

    // Array of keyboard keys being currently pressed
    KeyInputs _key_inputs;

    // FPS Timer
    FPS_Timer _fps_timer;

    // Sets the window's main monitor
    void _setMainMonitor(_internal::Monitor const& monitor);
};

class Context {
public:
    Context()                           = delete;   // Default constructor
    Context(std::weak_ptr<Window> ptr);             // Constructor
    Context(GLFWwindow* ptr);                       // Constructor
    ~Context();                                     // Destructor
    Context(const Context&)             = delete;   // Copy constructor
    Context(Context&&)                  = delete;   // Move constructor
    Context& operator=(const Context&)  = delete;   // Copy assignment
    Context& operator=(Context&&)       = delete;   // Move assignment
private:
    GLFWwindow* _given{ nullptr };
    GLFWwindow* _previous{ nullptr };
    bool _equal{ true };
};

__SSS_GL_END