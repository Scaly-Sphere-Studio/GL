#pragma once

#include "SSS/GL/internal/callbacks.hpp"
#include "SSS/GL/Objects/Shaders.hpp"
#include "SSS/GL/Objects/Texture.hpp"
#include "SSS/GL/Objects/Camera.hpp"
#include "SSS/GL/Objects/Models/Plane.hpp"
#include "SSS/GL/Objects/Models/PlaneRenderer.hpp"
#include "SSS/GL/Objects/Models/Line.hpp"
#include "SSS/GL/Objects/Models/LineRenderer.hpp"

/** @file
 *  Defines class SSS::GL::Window and its bound class SSS::GL::Context.
 */

namespace SSS::Log::GL {
    /** Logging properties for SSS::GL::Context.*/
    struct Context : public LogBase<Context> {
        using LOG_STRUCT_BASICS(Log, Context);
        bool set_context = false;
    };
    /** Logging properties for SSS::GL::Window.*/
    struct Window : public LogBase<Window> {
        using LOG_STRUCT_BASICS(Log, Window);
        bool glfw_init = false;
        bool life_state = false;
        bool fps = false;
        bool hovered_model = false;
    };
}

SSS_GL_BEGIN;
    
/** Global function which polls everything in the library.
 *  Process is as follow:
 * - Calls glfwPollEvents().
 * - Calls all TR::Area::update().
 * - Calls all Model's passive function.
 * - Edits every Texture with a pending thread (file load / text area).
 * - \b Returns \c true if at least one Window is visible, and \c false otherwise.
 */
bool pollEverything();

template <class T>
uint32_t getAvailableID(std::map<uint32_t, T> const& map)
{
    uint32_t id = 0;
    while (id < UINT32_MAX && map.count(id) != 0) {
        ++id;
    }
    return id;
}

/** Abstractization of \c GLFWwindow logic.*/
class Window final : public std::enable_shared_from_this<Window> {
    
    friend bool pollEverything();
    friend void _internal::window_iconify_callback(GLFWwindow* ptr, int state);
    friend void _internal::window_resize_callback(GLFWwindow* ptr, int w, int h);
    friend void _internal::window_pos_callback(GLFWwindow* ptr, int x, int y);
    friend void _internal::mouse_position_callback(GLFWwindow* ptr, double x, double y);
    friend void _internal::mouse_button_callback(GLFWwindow* ptr, int button, int action, int mods);
    friend void _internal::key_callback(GLFWwindow* ptr, int key, int scancode, int action, int mods);
    friend void _internal::monitor_callback(GLFWmonitor* ptr, int event);

public:
    /** Window::create() parameters.*/
    struct CreateArgs final {
        /** Width of the window (auto-shrinks to fit monitor).*/
        int w{ 720 };
        /** Height of the window (auto-shrinks to fit monitor).*/
        int h{ 720 };
        /** Title of the window.*/
        std::string title{ "Untitled" };
        /** Monitor ID of the window (auto-shrinks to closest valid value).*/
        int monitor_id{ 0 };
        /** Whether to put the window in fullscreen.*/
        bool fullscreen{ false };
        /** Whether to maximize the window.*/
        bool maximized{ false };
        /** Whether to iconify the window.*/
        bool iconified{ false };
        /** Whether to hide the window.*/
        bool hidden{ false };
    };

    /** %Shared instance, retrivable with Window::get().*/
    using Shared = std::shared_ptr<Window>;

private:
    using Weak = std::weak_ptr<Window>;         // Weak ptr to Window instance
    static std::vector<Weak> _instances;        // All Window instances
    static std::vector<GLFWmonitor*> _monitors; // All connected monitors

    // Constructor, creates a window
    // Private, to be called via Window::create();
    Window(CreateArgs const& args);

    // To be called in create(), as weak_from_this() is needed
    void _loadPresetShaders();

public :
    /** Destructor, calls cleanObjects(), removes instance from internal
     *  vector, and terminates glfw if no Window exists anymore.
     *  @sa create(), shouldClose()
     */
    ~Window();

    /** \cond INTERNAL*/
    // Rule of 5
    Window(const Window&)               = delete;   // Copy constructor
    Window(Window&&)                    = delete;   // Move constructor
    Window& operator=(const Window&)    = delete;   // Copy assignment
    Window& operator=(Window&&)         = delete;   // Move assignment
    /** \endcond*/

    /** Inits glfw if no Window exists yet, and creates a Shared
     *  instance with given parameters.
     *  @sa Window::get(), shouldClose(), ~Window()
     */
    static Shared create(CreateArgs const& args);
    /** Retrieves the Shared instance matching given GLFWwindow pointer.
     *  @sa Window::create()
     */
    static Shared get(GLFWwindow* ptr);

    static Shared getFirst();

    /** Returns \c true if the user requested to close the window,
     *  and \c false otherwiser.
     *  Effectively calls glfwWindowShouldClose().
     *  @sa ~Window()
     */
    bool shouldClose() const noexcept;
    /** Sets a corresponding callback.
     *  @usage
     *  @code
     *  void key_callback(GLFWwindow* ptr, int key, int scancode, int action, int mods)
     *  {
     *      Window::Shared window = Window::get(ptr);
     *      // Process input ...
     *  }
     *  
     *  int main()
     *  {
     *      // Setup window ...
     *      window->setCallback(glfwSetKeyCallback, key_callback);
     *  }
     *  @endcode
     *  @sa getKeyInputs()
     */
    template<typename Callback>
    void setCallback(Callback(*set)(GLFWwindow*, Callback), Callback callback);

    /** Array storing currently-pressed keys (see glfw key macros).
     *  @sa getKeyInputs()
     */
    using KeyInputs = std::array<bool, GLFW_KEY_LAST + 1>;
    /** Returns an array storing currently-pressed keys (see glfw key macros).
     *  @sa setCallback()
     */
    inline KeyInputs const& getKeyInputs() const noexcept { return _key_inputs; }

    /** Stores all Window-bound objects in ID maps.
     *  @sa Window::getObjects()
     */
    struct Objects final {
        /** \cond INTERNAL*/
        // Rule of 5
        Objects()                           = default;  // Constructor
        ~Objects()                          = default;  // Destructor
        Objects(const Objects&)             = delete;   // Copy constructor
        Objects(Objects&&)                  = delete;   // Move constructor
        Objects& operator=(const Objects&)  = delete;   // Copy assignment
        Objects& operator=(Objects&&)       = delete;   // Move assignment
        /** \endcond*/
        std::map<uint32_t, Shaders::Ptr> shaders;       // Shaders
        std::map<uint32_t, Renderer::Ptr> renderers;    // Renderers
        std::map<uint32_t, Texture::Ptr> textures;      // Textures
    };

private:
    Objects _objects;   // All Window-bound objects

public:
    /** Returns all Window-bound Objects.
     *  @sa cleanObjects()
     */
    inline Objects const& getObjects() const noexcept { return _objects; };
    /** Removes all objects from internal Window::Objects (except preset shaders).
     *  @sa getObjects()
     */
    void cleanObjects() noexcept;

    /** Creates a Shaders::Ptr in Objects::shaders at given ID (see Shaders::Preset).
     *  @sa removeShaders()
     */
    Shaders::Ptr const& createShaders(uint32_t id);
    Shaders::Ptr const& createShaders();
    /** Creates a derived Renderer::Ptr in Objects::renderers at given ID.
     *  @sa removeRenderer()
     */
    template<class Derived = Renderer>
    Renderer::Ptr const& createRenderer(uint32_t id);
    template<class Derived = Renderer>
    Renderer::Ptr const& createRenderer();
    /** Creates a Texture::Ptr in Objects::textures at given ID.
     *  @sa removeTexture()
     */
    Texture::Ptr const& createTexture(uint32_t id);
    Texture::Ptr const& createTexture();

    /** Removes the Shaders::Ptr in Objects::shaders at given ID (see Shaders::Preset).
     *  @sa createShaders()
     */
    void removeShaders(uint32_t id);
    /** Removes the Renderer::Ptr in Objects::renderers at given ID.
     *  @sa createRenderer()
     */
    void removeRenderer(uint32_t id);
    /** Removes the Texture::Ptr in Objects::textures at given ID.
     *  @sa createTexture()
     */
    void removeTexture(uint32_t id);

    /** Draws everything as defined in Objects::renderers.
     *  @sa printFrame()
     */
    void drawObjects();

    /** Swaps internal buffers (prints frame) if the instance is visible.
     *  Sleeps (if needed) to follow FPS limit, and updates mouse hovering status.
     *  @sa drawObjects()
     */
    void printFrame();

    inline void saveScreenshot() noexcept { take_screenshot = true; };

private:
    class AsyncScreenshot : public AsyncBase
        < int, int, std::vector<uint8_t>, std::string >
    {
        void _asyncFunction(int w, int h, std::vector<uint8_t> pixels,
            std::string filename);
    };
    using AsyncPair = std::pair<AsyncScreenshot, std::string>;
    std::list<AsyncPair> _screenshots;
    bool take_screenshot{ false };
    void _saveScreenshot();

    FrameTimer _frame_timer;
    std::chrono::steady_clock::time_point _last_render_time;
    std::chrono::steady_clock::duration _hover_waiting_time;
    bool _cursor_is_moving{ false };
    double _old_cursor_x{ 0 }, _old_cursor_y{ 0 };
    uint32_t _hovered_id{ 0 };
    enum class HoveredType {
        None = 0,
        Plane
    } _hovered_type{ HoveredType::None };
    void _updateHoveredModel();
    void _updateHoveredModelIfNeeded(std::chrono::steady_clock::time_point const& now);
    void _callPassiveFunctions();
    void _callOnClickFunction(int button, int action, int mods);

public:
    /** Returns the last computed FPS.
     *  @sa printFrame(), setFPSLimit()
     */
    inline long long getFPS() { return _frame_timer.get(); };
    /** Sets an FPS limit (<= 0 means no limit).
     *  @sa printFrame(), getFPS(), getFPSLimit()
     */
    void setFPSLimit(int fps_limit);
    /** Returns the current FPS limit (<= 0 means no limit).
     *  @sa getFPS(), setFPSLimit()
     */
    inline int getFPSLimit() const noexcept { return _fps_limit; };
    
    /** Enables or disable vertical synchronization (VSYNC).
     *  @sa getVSYNC()
     */
    void setVSYNC(bool state);
    /** Returns \c true if the vertical synchronization (VSYNC) is
     *  enabled, and \c false otherwise.
     *  @sa setVSYNC()
     */
    inline bool getVSYNC() const noexcept { return _vsync; };

    /** Sets the window title.
     *  @sa getTitle()
     */
    void setTitle(std::string const& title);
    /** Returns the current window title.
     *  @sa setTitle()
     */
    inline std::string const& getTitle() const noexcept { return _title; };

    /** Resizes the window to given dimensions.
     *  @sa getDimensions(), getRatio()
     */
    inline void setDimensions(int w, int h) { glfwSetWindowSize(_window.get(), w, h); };
    /** Returns the current window dimensions in given parameters.
     *  @sa setDimensions(), getRatio()
     */
    inline void getDimensions(int& w, int& h) const noexcept { w = _w; h = _h; };
    /** Returns the current window ratio (width / height).
     *  @sa setDimensions(), getDimensions()
     */
    inline float getRatio() const noexcept
        { return static_cast<float>(_w) / static_cast<float>(_h); }

    /** Moves the window to given screen coordinates.
     *  @sa getPosition()
     */
    inline void setPosition(int x0, int y0) { glfwSetWindowPos(_window.get(), x0, y0); };
    /** Returns the window's screen coordinates in given parameters.
     *  @sa getPosition()
     */
    inline void getPosition(int& x0, int& y0) const noexcept
        { glfwGetWindowPos(_window.get(), &x0, &y0); };

    /** Enables or disables fullscreen mode on given monitor.
     *  If \c monitor_id is negative, the window's main monitor is used.
     *  @sa isFullscreen()
     */
    void setFullscreen(bool state, int monitor_id = -1);
    /** Returns \c true if the window is in fullscreen mode,
     *  and \c false otherwise.
     *  @sa setFullscreen()
     */
    inline bool isFullscreen() const noexcept
        { return glfwGetWindowMonitor(_window.get()) != nullptr; };

    /** Iconifies or restores the window.
     *  @sa isIconified()
     */
    void setIconification(bool iconify);
    /** Returns \c true if the window is iconified, and \c false otherwise
     *  @sa setIconification()
     */
    inline bool isIconified() const noexcept { return _is_iconified; };

    /** Maximizes or restores the window.
     *  @sa isMaximized()
     */
    void setMaximization(bool maximize);
    /** Returns \c true if the window is maximized, and \c false otherwise
     *  @sa setMaximization()
     */
    bool isMaximized() const;

    /** Show (\c true) or hide (\c false) the window.
     *  @sa isVisible()
     */
    void setVisibility(bool show);
    /** Returns \c true if the window is visible, and \c false otherwise.
     *  @sa setVisibility()
     */
    bool isVisible() const;

    /** Returns the corresponding \c GLFWwindow pointer, be careful with it.*/
    inline GLFWwindow* getGLFWwindow() const { return _window.get(); };

    inline uint32_t maxGLSLTextureUnits() const noexcept { return _max_glsl_tex_units; };

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
    // Max GLSL texture units
    uint32_t _max_glsl_tex_units;
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

template<class Derived>
inline Renderer::Ptr const& Window::createRenderer(uint32_t id)
{
    Renderer::Ptr& ptr = _objects.renderers[id];
    ptr.reset(new Derived(weak_from_this(), id));
    return ptr;
}

template<class Derived>
inline Renderer::Ptr const& Window::createRenderer()
{
    try {
        return createRenderer<Derived>(getAvailableID(_objects.renderers));
    }
    catch (std::exception const& e) {
        static Renderer::Ptr n(nullptr);
        LOG_FUNC_ERR(e.what());
        return n;
    }
}

template<typename Callback>
inline void Window::setCallback(Callback(*set)(GLFWwindow*, Callback), Callback callback)
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

template<class T>
Renderer::Ptr const& Renderer::create(std::shared_ptr<Window> win)
{
    if (!win) {
        win = Window::getFirst();
    }
    return win->createRenderer<T>();
}

/** Abstractization of glfw contexts, inspired by std::lock.
 *  Make given context current in scope.
 *  @usage
 *  @code
 *  // Context is set to something
 *  
 *  void func(Window::Shared window)
 *  {
 *      // Set context to given window
 *      Context const context(window);
 *      
 *      // OpenGL operations ...
 *  }
 * 
 *  // Context is back to previous
 *  @endcode
 */
class Context final {
private:
    void _init(GLFWwindow* ptr);
public:
    /** Make given context current if needed.*/
    Context(std::weak_ptr<Window> ptr);
    /** Make given context current if needed.*/
    Context(GLFWwindow* ptr);
    /** Swap to previous context if it was changed in constructor.*/
    ~Context();
    /** @cond INTERNAL*/
    Context()                           = delete;   // Default constructor
    Context(const Context&)             = delete;   // Copy constructor
    Context(Context&&)                  = delete;   // Move constructor
    Context& operator=(const Context&)  = delete;   // Copy assignment
    Context& operator=(Context&&)       = delete;   // Move assignment
    /** @endcond*/
private:
    GLFWwindow* _given{ nullptr };
    GLFWwindow* _previous{ nullptr };
    bool _equal{ true };
};

SSS_GL_END;