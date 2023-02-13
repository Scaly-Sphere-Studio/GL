#include "SSS/GL/Window.hpp"

SSS_GL_BEGIN;

// Window ptr
std::vector<std::weak_ptr<Window>> Window::_instances{};
// Connected monitors
std::vector<GLFWmonitor*> Window::_monitors{};

// Constructor, creates a window
Window::Window(CreateArgs const& args) try
{
    // Init GLFW
    if (_instances.empty()) {
        // Init GLFW
        glfwInit();
        // Log
        if (Log::GL::Window::query(Log::GL::Window::get().glfw_init)) {
            LOG_GL_MSG("GLFW initialized");
        }
        // Retrive monitors
        monitor_callback(nullptr, 0);
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
        nullptr
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

    // Make context current for this scope
    Context const context(_window.get());

    // Init glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        SSS::throw_exc("Failed to initialize GLAD");
    }

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
    
    // Retrieve max number of GLSL texture units
    int max_units;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_units);
    _max_glsl_tex_units = static_cast<uint32_t>(max_units);

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
    // Free all bound objects
    _preset_shaders.clear();
    _renderers.clear();
    // Remove weak ptr from instance vector
    cleanWeakPtrVector(_instances);
    // Terminate GLFW
    if (_instances.empty()) {
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

Window::Shared Window::create(CreateArgs const& args) try
{
    Shared ptr(new Window(args));
    _instances.emplace_back(ptr);
    ptr->_loadPresetShaders();
    return ptr;
}
CATCH_AND_RETHROW_FUNC_EXC;

Window::Shared Window::get(GLFWwindow* ptr) try
{
    for (Weak const& weak : _instances) {
        Shared window = weak.lock();
        if (window && window->_window.get() == ptr) {
            return window;
        }
    }
    throw_exc("Found no window for given pointer.");
}
CATCH_AND_RETHROW_FUNC_EXC;

Window::Shared Window::getFirst()
{
    if (_instances.empty()) {
        return Shared(nullptr);
    }
    return _instances[0].lock();
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
        key = Input::None;
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
    // Make context current for this scope
    Context const context(_window.get());
    // Set VSYNC
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
        if (monitor_id >= static_cast<int>(_monitors.size())) {
            LOG_METHOD_WRN("monitor_id out of range.");
            return;
        }
        // Select monitor
        GLFWmonitor* monitor = monitor_id < 0 ? _main_monitor : _monitors[monitor_id];
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
    if (id >= _monitors.size()) {
        id = static_cast<int>(_monitors.size() - 1);
    }
    _main_monitor_id = id;
    _main_monitor = _monitors[id];
}

void Window::_callPassiveFunctions()
{
    Shared win = shared_from_this();
    if (!win) return;

    // Call Planes' passive functions
    for (Plane::Weak const& weak : Plane::_instances) {
        Plane::Shared plane = weak.lock();
        uint32_t const func_id = plane->_passive_func_id;
        if (func_id == 0 || Plane::passive_funcs.count(func_id) == 0) {
            continue;
        }
        Plane::PassiveFunc const f = Plane::passive_funcs.at(func_id);
        if (f != nullptr) {
            f(win, plane);
        }
    }
}

void Window::_callOnClickFunction(int button, int action, int mods)
{
    Shared win = shared_from_this();
    if (!win) return;

    uint32_t const& id = _hovered_id;
    switch (_hovered_type) {
    case HoveredType::None:
        break;
    case HoveredType::Plane: {
        Plane::Shared plane = Plane::getHovered();
        if (!plane)
            break;
        uint32_t const func_id = plane->_on_click_func_id;
        if (func_id == 0 || Plane::on_click_funcs.count(func_id) == 0) {
            break;
        }
        Plane::OnClickFunc const f = Plane::on_click_funcs.at(func_id);
        if (f != nullptr) {
            f(win, plane, button, action, mods);
        }
        break;
    }
    default:
        break;
    }
}

SSS_GL_END;