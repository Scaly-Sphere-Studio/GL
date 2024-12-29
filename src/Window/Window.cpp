#include "GL/Window.hpp"
#include "GL/Objects/Camera.hpp"
#include "GL/Objects/Models/Plane.hpp"

SSS_GL_BEGIN;

Window::MainPtr Window::_main;

std::function<void()> _internal::gladLoader;

// Constructor, creates a window
Window::Window(CreateArgs const& args) try
    : _is_main(!_main)
{
    if (_is_main) {
        // Init GLFW
        glfwInit();
        // Log
        if (Log::GL::Window::query(Log::GL::Window::get().glfw_init)) {
            LOG_GL_MSG("GLFW initialized");
        }

        _retrieveMonitors();
        glfwSetMonitorCallback(monitor_callback);
    }

    // Set main monitor
    _setMainMonitor(args.monitor_id);
    // Retrieve video size of monitor and adjust size parameters
    GLFWvidmode const* mode = glfwGetVideoMode(_main_monitor);
    if (args.fullscreen && !args.hidden) {
        _w = mode->width;
        _h = mode->height;
    }
    else {
        _w = args.w < mode->width ? args.w : mode->width;
        _h = args.h < mode->height ? args.h : mode->height;
    }
    _title = args.title;

    // If in Debug mode, set the Debug hint
    if constexpr (DEBUGMODE) {
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    }
    // Hints
    glfwWindowHint(GLFW_VISIBLE, args.hidden ? GLFW_FALSE : GLFW_TRUE);
    // Create window
    _window.reset(glfwCreateWindow(
        _w,
        _h,
        _title.c_str(),
        (args.fullscreen && !args.hidden) ? _main_monitor : nullptr,
        _is_main ? nullptr : _main->getGLFWwindow()
    ));
    // Throw if an error occured
    if (!_window.get()) {
        const char* msg;
        glfwGetError(&msg);
        throw_exc(msg);
    }

    int x, y;
    glfwGetMonitorPos(_main_monitor, &x, &y);
    // Center window on given monitor if needed
    if (!args.fullscreen && !args.hidden) {
        glfwSetWindowPos(_window.get(), x + (mode->width - _w) / 2, y + (mode->height - _h) / 2);
        // Maximize if needed
        if (args.maximized) {
            setMaximization(args.maximized);
        }
    }
    // Retrieve actual size & pos
    glfwGetWindowSize(_window.get(), &_w, &_h);
    glfwGetWindowPos(_window.get(), &_x, &_y);
    // Compute windowed pos
    _windowed_x = x + (mode->width - _w) / 2;
    _windowed_y = y + (mode->height - _h) / 2;
    // Iconify if needed
    if (args.iconified && !args.hidden) {
        setIconification(args.iconified);
    }

    glfwMakeContextCurrent(_window.get());

    // Init glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        SSS::throw_exc("Failed to initialize GLAD");
    }
    if (_internal::gladLoader)
        _internal::gladLoader();

    // Set viewport
    glViewport(0, 0, _w, _h);
    // Enable blending (transparency)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initial clear
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(_window.get());

    // Set VSYNC to false by default
    setVSYNC(false);

    // Set window callbacks, after everything else is set
    glfwSetWindowIconifyCallback(_window.get(), window_iconify_callback);
    glfwSetWindowSizeCallback(_window.get(), window_resize_callback);
    glfwSetWindowPosCallback(_window.get(), window_pos_callback);
    glfwSetCursorPosCallback(_window.get(), mouse_position_callback);
    glfwSetMouseButtonCallback(_window.get(), mouse_button_callback);
    glfwSetKeyCallback(_window.get(), key_callback);
    glfwSetCharCallback(_window.get(), char_callback);
    
    if (_is_main) {
        // Retrieve max number of GLSL texture units
        int max_units;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_units);
        _main._max_glsl_tex_units = static_cast<uint32_t>(max_units);

        _loadPresetShaders();
    }
    else {
        glfwMakeContextCurrent(_main->getGLFWwindow());
    }

    // Log
    if (Log::GL::Window::query(Log::GL::Window::get().life_state)) {
        char buff[256];
        sprintf_s(buff, "'%s' -> created", _title.c_str());
        LOG_GL_MSG(buff);
    }
}
CATCH_AND_RETHROW_METHOD_EXC;

// Destructor
Window::~Window()
{
    _renderers.clear();
    if (!_is_main) {
        if (_main._subs.count(_window.get()) != 0 && !_main._subs[_window.get()]) {
            _main._subs.erase(_window.get());
        }
        _window.reset();
    }
    else {
        _main._preset_shaders.clear();
        _main._subs.clear();
        _window.reset();
        // Terminate GLFW
        glfwTerminate();
        // Log
        if (Log::GL::Window::query(Log::GL::Window::get().glfw_init)) {
            LOG_GL_MSG("GLFW terminated");
        }
    }
    // Log
    if (Log::GL::Window::query(Log::GL::Window::get().life_state)) {
        char buff[256];
        sprintf_s(buff, "'%s' -> deleted", _title.c_str());
        LOG_GL_MSG(buff);
    }
}

Window& Window::create(CreateArgs const& args)
{
    if (!_main) {
        _main.reset(new Window(args));
        return *_main;
    }
    Ptr win;
    win.reset(new Window(args));
    GLFWwindow* ptr = win->getGLFWwindow();
    _main._subs[ptr] = std::move(win);
    for (auto& weak_camera : Camera::_instances) {
        auto camera = weak_camera.lock();
        if (camera)
            camera->_computeProjections();
    }
    return *_main._subs[ptr];
}

Window* Window::get(GLFWwindow* ptr) noexcept
{
    if (_main->getGLFWwindow() == ptr)
        return _main.get();
    for (auto& [key, win] : _main._subs) {
        if (key == ptr)
            return win.get();
    }
    return nullptr;
}

Window* Window::getCurrent() noexcept
{
    return get(glfwGetCurrentContext());
}

std::vector<Window*> Window::getAll() noexcept
{
    std::vector<Window*> ret;
    ret.emplace_back(_main.get());
    for (auto& [ptr, win] : _main._subs)
        ret.emplace_back(win.get());
    return ret;
}

void Window::close()
{
    if (_is_main)
        _main.reset();
    else
        _main._subs.erase(_window.get());
}

Context const Window::setContext()
{
    return Context(_window.get());
}

// Wether the user requested to close the window.
// NOTE: this simply is a call to glfwWindowShouldClose
bool Window::shouldClose() const noexcept
{
    return glfwWindowShouldClose(_window.get());
}

void Window::blockInputs(int unblocking_key) noexcept
{
    _block_inputs = true;
    _unblocking_key = unblocking_key;
    for (auto& key : _key_inputs) {
        key.reset();
    }
}

void Window::setFPSLimit(int fps_limit)
{
    _fps_limit = fps_limit;
    if (_fps_limit <= 0) {
        _fps_limit = 0;
        _min_frame_time = std::chrono::nanoseconds(0);
    }
    else {
        if (_fps_limit > 1000) {
            _fps_limit = 1000;
        }
        _min_frame_time = std::chrono::nanoseconds
            (1000000000ll / static_cast<long long>(fps_limit));
    }
}

// Enables or disables the VSYNC of the window
void Window::setVSYNC(bool state)
{
    Context const context = setContext();
    _vsync = state;
    glfwSwapInterval(_vsync);
}

// Enables or disables fullscreen mode on given screen
// -> monitor_id of -1 (default) takes the monitor the window is on
void Window::setFullscreen(bool state, int monitor_id)
{
    // Retrieve current fullscreen monitor (nullptr when windowed)
    GLFWmonitor* fullscreenMonitor = glfwGetWindowMonitor(_window.get());

    if (state) {
        // Ensure given ID is in range
        if (monitor_id >= static_cast<int>(_main._monitors.size())) {
            LOG_METHOD_WRN("monitor_id out of range.");
            return;
        }
        // Select monitor
        GLFWmonitor* monitor = monitor_id < 0 ? _main_monitor : _main._monitors[monitor_id];
        // Ensure window isn't already fullscreen on given ID
        if (fullscreenMonitor == monitor) {
            LOG_METHOD_WRN("window is already fullscreen on given screen");
            return;
        }
        // Store current size & pos, if currently in windowed mode
        if (fullscreenMonitor == nullptr) {
            glfwGetWindowPos(_window.get(), &_windowed_x, &_windowed_y);
        }
        // Set window in fullscreen
        glfwSetWindowMonitor(_window.get(), monitor, 0, 0, _w, _h, GLFW_DONT_CARE);
    }
    else {
        // Ensure the window isn't arealdy windowed
        if (fullscreenMonitor == nullptr) {
            LOG_METHOD_WRN("window is already windowed.");
            return;
        }
        // Set window in windowed mode with old values
        glfwSetWindowMonitor(_window.get(), nullptr,
            _windowed_x, _windowed_y, _w, _h, GLFW_DONT_CARE);
    }
}

void Window::setIconification(bool iconify)
{
    if (iconify) {
        glfwIconifyWindow(_window.get());
    }
    else {
        glfwRestoreWindow(_window.get());
    }
}

void Window::setMaximization(bool maximize)
{
    if (maximize) {
        glfwMaximizeWindow(_window.get());
    }
    else {
        glfwRestoreWindow(_window.get());
    }
}

bool Window::isMaximized() const
{
    return glfwGetWindowAttrib(_window.get(), GLFW_MAXIMIZED);
}

void Window::setVisibility(bool show)
{
    if (show) {
        glfwShowWindow(_window.get());
    }
    else {
        glfwHideWindow(_window.get());
    }
}

bool Window::isVisible() const
{
    return static_cast<bool>(glfwGetWindowAttrib(_window.get(), GLFW_VISIBLE));
}

void Window::setTitle(std::string const& title)
{
    _title = title;
    glfwSetWindowTitle(_window.get(), title.c_str());
}

void Window::_setMainMonitor(int id)
{
    if (id < 0) {
        id = 0;
    }
    if (id >= _main._monitors.size()) {
        id = static_cast<int>(_main._monitors.size() - 1);
    }
    _main_monitor_id = id;
    _main_monitor = _main._monitors[id];
}

SSS_GL_END;