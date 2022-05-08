#pragma once

#include "_internal/callbacks.hpp"
#include "Texture.hpp"
#include "Camera.hpp"
#include "Plane.hpp"
#include "_internal/PlaneRenderer.hpp"

/** @file
 *  Defines class SSS::GL::Window and its bound class SSS::GL::Context.
 */

namespace SSS::Log::GL {
    /** Logging properties for internal SSS::GL::Shaders.*/
    struct Context : public LogBase<Context> {
        using LOG_STRUCT_BASICS(Log, Context);
        bool set_context = false;
    };
}

SSS_GL_BEGIN;
    
class Window : public std::enable_shared_from_this<Window> {
    
    friend bool pollEverything();
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
    
    /** \cond TODO*/
    struct LOG {
        static bool constructor;
        static bool destructor;
        static bool glfw_init;
        static bool fps;
        static bool longest_frame;
    };
    /** \endcond*/

    struct CreateArgs {
        int w{ 720 };
        int h{ 720 };
        std::string title{ "Untitled" };
        int monitor_id{ 0 };
        bool fullscreen{ false };
        bool maximized{ false };
        bool iconified{ false };
        bool hidden{ false };
    };

// --- Public aliases ---

    using Shared = std::shared_ptr<Window>;

private:
    using Weak = std::weak_ptr<Window>;
    // All Window instances
    static std::vector<Weak> _instances;
    // All connected monitors
    static std::vector<GLFWmonitor*> _monitors;

    // Constructor, creates a window
    // Private, to be called via Window::create();
    Window(CreateArgs const& args);

public :
    // Rule of 5
    ~Window();                                      // Destructor
    Window(const Window&)               = delete;   // Copy constructor
    Window(Window&&)                    = delete;   // Move constructor
    Window& operator=(const Window&)    = delete;   // Copy assignment
    Window& operator=(Window&&)         = delete;   // Move assignment

    static Shared create(CreateArgs const& args);
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
        std::map<uint32_t, Shaders::Ptr> shaders;       // Shaders
        std::map<uint32_t, Renderer::Ptr> renderers;    // Renderers
        std::map<uint32_t, Plane::Ptr> planes;          // Planes
        std::map<uint32_t, Texture::Ptr> textures;      // Textures
        std::map<uint32_t, Camera::Ptr> cameras;        // Cameras
    };

private:
    // Stores all window objects
    Objects _objects;
    // To be called in create() as weak_from_this() is needed
    void loadPreSetShaders();

public:
    inline Objects const& getObjects() const noexcept { return _objects; };
    void cleanObjects() noexcept;

    void createShaders(uint32_t id);
    void removeShaders(uint32_t id);

    template<class T = Renderer>
    void createRenderer(uint32_t id) {
        _objects.renderers.try_emplace(id);
        _objects.renderers.at(id).reset(new T(weak_from_this()));
    }
    void removeRenderer(uint32_t id);
    void createModel(uint32_t id, Model::Type type);
    void removeModel(uint32_t id, Model::Type type);

    void createTexture(uint32_t id);
    void removeTexture(uint32_t id);

    void createCamera(uint32_t id);
    void removeCamera(uint32_t id);


// --- Public methods ---

    // Draws objects inside renderers on the back buffer.
    void drawObjects();
    // Renders back buffer, clears front buffer, polls events.
    // Logs fps if specified in LOG structure.
    void printFrame();
private:
    FrameTimer _frame_timer;
    std::chrono::steady_clock::time_point _last_render_time;
    std::chrono::steady_clock::duration _hover_waiting_time;
    bool _cursor_is_moving{ false };
    double _old_cursor_x{ 0 }, _old_cursor_y{ 0 };
    bool _something_is_hovered{ false };
    uint32_t _hovered_model_id{ 0 };
    Model::Type _hovered_model_type{ Model::Type::Plane };
    void _updateHoveredModel();
    void _updateHoveredModelIfNeeded(std::chrono::steady_clock::time_point const& now);
    void _callPassiveFunctions();

public:
    inline long long getFPS() { return _frame_timer.get(); };

    // Wether the user requested to close the window.
    // NOTE: this simply is a call to glfwWindowShouldClose
    bool shouldClose() const noexcept;

    void setFPSLimit(int fps_limit);
    inline int getFPSLimit() const noexcept { return _fps_limit; };
    
    // Enables or disables the VSYNC of the window
    void setVSYNC(bool state);
    inline bool getVSYNC() const noexcept { return _vsync; };
    
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
    inline GLFWmonitor* getMonitor() const noexcept { return _main_monitor; }

    void setTitle(std::string const& title);
    inline std::string const& getTitle() const noexcept { return _title; };

    inline void setDimensions(int w, int h) { glfwSetWindowSize(_window.get(), w, h); };
    inline void getDimensions(int& w, int& h) const noexcept { w = _w; h = _h; };
    inline float getScreenRatio() const noexcept
        { return static_cast<float>(_w) / static_cast<float>(_h); }

    inline void setPosition(int x0, int y0) { glfwSetWindowPos(_window.get(), x0, y0); };
    inline void getPosition(int& x0, int& y0) const noexcept
        { glfwGetWindowPos(_window.get(), &x0, &y0); };

    // Enables or disables fullscreen mode on given screen
    void setFullscreen(bool state, int screen_id = -1);
    inline bool isFullscreen() const noexcept
        { return glfwGetWindowMonitor(_window.get()) != nullptr; };

    void setIconification(bool iconify);
    inline bool isIconified() const noexcept { return _is_iconified; };

    void setMaximization(bool maximize);
    bool isMaximized() const;

    void setVisibility(bool show);
    bool isVisible() const;

private:
// --- Private variables ---

    // Window title
    std::string _title;
    // Window size
    int _w; // Width
    int _h; // Height
    // Windowed to Fullscreen variables
    int _windowed_x{ 0 };   // Old x (left) pos
    int _windowed_y{ 0 };   // Old y (up) pos


    // FPS Limit (0 = disabled)
    int _fps_limit{ 0 };
    std::chrono::nanoseconds _min_frame_time{ 0 };
    // VSYNC state
    bool _vsync{ false };
    // Iconify state
    bool _is_iconified{ false };

    // GLFWwindow ptr
    C_Ptr <GLFWwindow, void(*)(GLFWwindow*), glfwDestroyWindow> _window;
    // Main monitor the window is on
    GLFWmonitor* _main_monitor;
    int _main_monitor_id{ 0 };

    // Internal callbacks that are called before calling within themselves user callbacks
    GLFWwindowiconifyfun _iconify_callback{ nullptr };          // Window iconify
    GLFWwindowsizefun    _resize_callback{ nullptr };           // Window resize
    GLFWwindowposfun     _pos_callback{ nullptr };              // Window position
    GLFWkeyfun           _key_callback{ nullptr };              // Window keyboard key
    GLFWcursorposfun     _mouse_position_callback{ nullptr };   // Window mouse position
    GLFWmousebuttonfun   _mouse_button_callback{ nullptr };     // Window mouse button

    // Array of keyboard keys being currently pressed
    KeyInputs _key_inputs;

    // Sets the window's main monitor
    void _setMainMonitor(int id);
};

INTERNAL_BEGIN;
inline char const* windowTitle(Window::Shared win) noexcept
{
    if (win) {
        return win->getTitle().c_str();
    }
    return nullptr;
}

inline char const* windowTitle(std::weak_ptr<Window> win) noexcept
{
    return windowTitle(win.lock());
}
#define WINDOW_TITLE(X) _internal::windowTitle(X)

INTERNAL_END;


class Context {
private:
    void _init(GLFWwindow* ptr);
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

SSS_GL_END;