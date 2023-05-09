#ifndef SSS_GL_WINDOW_HPP
#define SSS_GL_WINDOW_HPP

#include "Objects/Basic.hpp"
#include <map>
#include <array>
#include <queue>

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
    /** Logging properties for internal SSS::GL::Window callbacks.*/
    struct Callbacks : public LogBase<Callbacks> {
        using LOG_STRUCT_BASICS(Log, Callbacks);
        bool window_resize = false;
        bool window_pos = false;
        bool window_iconify = false;
        bool mouse_position = false;
        bool mouse_button = false;
        bool key = false;
        bool monitor = false;
    };
}

SSS_GL_BEGIN;

class Shaders;
class RendererBase;
class ModelBase;

class SSS_GL_API Context {
    friend class Window;
public:
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
    /** Make given context current if needed.*/
    Context(GLFWwindow* ptr);
    GLFWwindow* _given{ nullptr };
    GLFWwindow* _previous{ nullptr };
    bool _equal{ true };
};

// Ignore warning about STL exports as they're private members
#pragma warning(push, 2)
#pragma warning(disable: 4251)
#pragma warning(disable: 4275)

/** Abstractization of \c GLFWwindow logic.*/
class SSS_GL_API Window : public Base, public std::enable_shared_from_this<Window> {
    
    friend SSS_GL_API void pollEverything();

private:
    static void window_iconify_callback(GLFWwindow* ptr, int state);
    static void window_resize_callback(GLFWwindow* ptr, int w, int h);
    static void window_pos_callback(GLFWwindow* ptr, int x, int y);
    static void mouse_position_callback(GLFWwindow* ptr, double x, double y);
    static void mouse_button_callback(GLFWwindow* ptr, int button, int action, int mods);
    static void key_callback(GLFWwindow* ptr, int key, int scancode, int action, int mods);
    static void char_callback(GLFWwindow* ptr, unsigned int codepoint);
    static void monitor_callback(GLFWmonitor* ptr, int event);

    static void _loadPresetShaders();
    static void _retrieveMonitors();

    class Ptr : public std::unique_ptr<Window> {
    public:
        _NODISCARD pointer operator->() const noexcept {
            pointer ret = get();
            if (ret == nullptr) {
                LOG_ERR("Trying to dereference Window when it hasn't been created or has since been closed.");
                abort();
            }
            return ret;
        }
    };

    // Additional static data
    class MainPtr : public Ptr {
    public:
        ~MainPtr() { reset(); };
        std::map<GLFWwindow*, Ptr> _subs;
        std::map<uint32_t, std::shared_ptr<Shaders>> _preset_shaders;
        std::vector<GLFWmonitor*> _monitors;
        uint32_t _max_glsl_tex_units{ 0 };
    };

    bool _is_main;
    static MainPtr _main;

public:
    struct CreateArgs {
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

    Window() = delete;
    /** Destructor, clears stored objects, removes instance from internal
     *  vector, and terminates glfw if no Window exists anymore.
     *  @sa create(), shouldClose()
     */
    ~Window();

private:
    // Constructor, creates a window
    // Private, to be called via Window::create();
    Window(CreateArgs const& args);

public:

    static Window& create(CreateArgs const& args = CreateArgs());

    static Window* get(GLFWwindow* ptr) noexcept;
    static Window* getCurrent() noexcept;
    static std::vector<Window*> getAll() noexcept;

    void close();

    Context const setContext();

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

private:
    // Internal callbacks that are called before calling within themselves user callbacks
    GLFWwindowiconifyfun _iconify_callback{ nullptr };          // Window iconify
    GLFWwindowsizefun    _resize_callback{ nullptr };           // Window resize
    GLFWwindowposfun     _pos_callback{ nullptr };              // Window position
    GLFWkeyfun           _key_callback{ nullptr };              // Window keyboard key
    GLFWcharfun          _char_callback{ nullptr };             // Window character input
    GLFWcursorposfun     _mouse_position_callback{ nullptr };   // Window mouse position
    GLFWmousebuttonfun   _mouse_button_callback{ nullptr };     // Window mouse button

public:

    /** Blocks all mouse & key inputs, with optional unblocking key.
     *  @sa unblockInputs(), areInputsBlocked()
     */
    void blockInputs(int unblocking_key = 0) noexcept;
    /** Reverts previous call to blockInputs().
     *  @sa areInputsBlocked()
     */
    inline void unblockInputs() noexcept { _block_inputs = false; };
    /** Whether mouse & key inputs are blocked.
     *  @sa blockInputs(), unblockInputs()
     */
    inline bool areInputsBlocked() const noexcept { return _block_inputs; };

    inline auto const& getKeyInputs() const noexcept { return _key_inputs; };

    inline void setInputStackTime(std::chrono::milliseconds ms) { _input_stack_time = ms; };
    inline auto getInputStackTime() const noexcept { return _input_stack_time; };


    inline bool keyIsHeld(int key) const noexcept { return _key_inputs[key].is_held(); };
    inline bool keyIsHeld(int key, int min_count) const noexcept
        { return _key_inputs[key].is_held(min_count); };

    inline bool keyIsPressed(int key) const noexcept { return _key_inputs[key].is_pressed(); };
    inline bool keyIsPressed(int key, int min_count) const noexcept
        { return _key_inputs[key].is_pressed(min_count); };

    inline bool keyIsReleased(int key) const noexcept { return _key_inputs[key].is_released(); };

    inline int keyCount(int key) const noexcept { return _key_inputs[key].count(); }


    inline auto const& getClickInputs() const noexcept { return _click_inputs; };

    inline bool clickIsHeld(int button) const noexcept { return _click_inputs[button].is_held(); };
    inline bool clickIsHeld(int button, int min_count) const noexcept
        { return _click_inputs[button].is_held(min_count); };

    inline bool clickIsPressed(int button) const noexcept { return _click_inputs[button].is_pressed(); };
    inline bool clickIsPressed(int button, int min_count) const noexcept
        { return _click_inputs[button].is_pressed(min_count); };

    inline bool clickIsReleased(int button) const noexcept { return _click_inputs[button].is_released(); };

    inline int clickCount(int button) const noexcept { return _click_inputs[button].count(); }

    inline void getCursorPos(int& x, int& y) const noexcept { x = _cursor_x; y = _cursor_y; };
    inline auto getCursorPos() const noexcept { return std::make_tuple(_cursor_x, _cursor_y); };
    inline void getCursorDiff(int& x, int& y) const noexcept { x = _cursor_diff_x; y = _cursor_diff_y; };
    inline auto getCursorDiff() const noexcept { return std::make_tuple(_cursor_diff_x, _cursor_diff_y); };

private:
    std::vector<std::shared_ptr<RendererBase>> _renderers; // Renderers

public:

    static std::shared_ptr<Shaders> getPresetShaders(uint32_t id) noexcept;

    inline void setRenderers(std::vector<std::shared_ptr<RendererBase>> const& renderers) noexcept { _renderers = renderers; };
    inline auto const& getRenderers() const noexcept { return _renderers; };
    void addRenderer(std::shared_ptr<RendererBase> renderer, size_t index);
    void addRenderer(std::shared_ptr<RendererBase> renderer);
    void removeRenderer(std::shared_ptr<RendererBase> renderer);

    std::shared_ptr<ModelBase> getHovered() const noexcept { return _hovered_model.lock(); };
    template<class Derived>
    std::shared_ptr<Derived> getHovered() const noexcept {
        std::shared_ptr<ModelBase> model = getHovered();
        if (model == nullptr)
            return nullptr;
        return std::dynamic_pointer_cast<Derived>(model);
    }

    std::shared_ptr<ModelBase> getClicked() const { return _clicked_model.lock(); };
    template<class Derived>
    std::shared_ptr<Derived> getClicked() const {
        std::shared_ptr<ModelBase> const model = getClicked();
        if (!model)
            return nullptr;
        return std::dynamic_pointer_cast<Derived>(model);
    }

    std::shared_ptr<ModelBase> getHeld() const { return _held_model.lock(); };
    template<class Derived>
    std::shared_ptr<Derived> getHeld() const {
        std::shared_ptr<ModelBase> const model = getHeld();
        if (!model)
            return nullptr;
        return std::dynamic_pointer_cast<Derived>(model);
    }

    /** Draws everything in order of Renderer IDs.
     *  @sa printFrame()
     */
    void drawObjects();

    /** Swaps internal buffers (prints frame) if the instance is visible.
     *  Sleeps (if needed) to follow FPS limit, and updates mouse hovering status.
     *  @sa drawObjects(), saveScreenshot()
     */
    void printFrame();

    /** Save a screenshot after drawing the next frame.
     *  Screenshot name is time-based.
     *  @sa printFrame()
     */
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
    bool _cursor_is_moving{ false }; // (10ms tolerance)
    
    int _cursor_x{ 0 }, _old_cursor_x{ 0 }, _cursor_diff_x{ 0 },
        _cursor_y{ 0 }, _old_cursor_y{ 0 }, _cursor_diff_y{ 0 };

    std::weak_ptr<ModelBase> _hovered_model;
    std::weak_ptr<ModelBase> _clicked_model;    // (left click)
    std::weak_ptr<ModelBase> _held_model;       // (left click)
    void _updateHoveredModel();
    void _updateHoveredModelIfNeeded(std::chrono::steady_clock::time_point const& now);

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
    inline void setWidth(int w) { setDimensions(w, _h); };
    inline void setHeight(int h) { setDimensions(_w, h); };
    /** Returns the current window dimensions in given parameters.
     *  @sa setDimensions(), getRatio()
     */
    inline void getDimensions(int& w, int& h) const noexcept { w = _w; h = _h; };
    inline std::tuple<int, int> getDimensions() const noexcept { return std::make_tuple(_w, _h); };
    inline int getWidth() const noexcept { return _w; };
    inline int getHeight() const noexcept { return _h; };
    /** Returns the current window ratio (width / height).
     *  @sa setDimensions(), getDimensions()
     */
    inline float getRatio() const noexcept
        { return static_cast<float>(_w) / static_cast<float>(_h); }

    /** Moves the window to given screen coordinates.
     *  @sa getPosition()
     */
    inline void setPosition(int x0, int y0) { glfwSetWindowPos(_window.get(), x0, y0); };
    inline void setPosX(int x) { setPosition(x, _y); };
    inline void setPosY(int y) { setPosition(_x, y); };
    /** Returns the window's screen coordinates in given parameters.
     *  @sa getPosition()
     */
    inline void getPosition(int& x0, int& y0) const noexcept { x0 = _x; y0 = _y; };
    inline std::tuple<int, int> getPosition() const noexcept { return std::make_tuple(_x, _y); };
    inline int getPosX() const noexcept { return _x; };
    inline int getPosY() const noexcept { return _y; };

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

    /** Returns the maximum number of texture units a GLSL fragment shader can hold.
     *  Basically just calls glGet() with GL_MAX_TEXTURE_IMAGE_UNITS.
     */
    static inline uint32_t maxGLSLTextureUnits() noexcept { return _main._max_glsl_tex_units; };

private:
// --- Private variables ---

    // Window title
    std::string _title;
    // Window size
    int _w; // Width
    int _h; // Height
    // Window pos
    int _x{ 0 }; // x (left) pos
    int _y{ 0 }; // y (up) pos
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

    // Whether to block inputs or not
    bool _block_inputs{ false };
    // Key to unblock inputs
    int _unblocking_key{ 0 };
    // Maximum delay for inputs to be considered stacked (eg: double click)
    std::chrono::milliseconds _input_stack_time{ 250 };

    std::queue<std::pair<int, bool>> _key_queue;
    std::array<Input, GLFW_KEY_LAST + 1> _key_inputs;
    
    std::queue<std::pair<int, bool>> _click_queue;
    std::array<Input, GLFW_MOUSE_BUTTON_LAST + 1> _click_inputs;

    void _poll();

    // Sets the window's main monitor
    void _setMainMonitor(int id);
};

#pragma warning(pop)

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
    else if (ptr == (void*)(&glfwSetCharCallback)) {
        _char_callback = GLFWcharfun(callback);
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

SSS_GL_END;

#endif // SSS_GL_WINDOW_HPP