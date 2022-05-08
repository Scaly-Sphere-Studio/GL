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
        _internal::monitor_callback(nullptr, 0);
        glfwSetMonitorCallback(_internal::monitor_callback);
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
    // Retrieve actual width & height
    glfwGetWindowSize(_window.get(), &_w, &_h);
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
    glfwSetWindowIconifyCallback(_window.get(), _internal::window_iconify_callback);
    glfwSetWindowSizeCallback(_window.get(), _internal::window_resize_callback);
    glfwSetWindowPosCallback(_window.get(), _internal::window_pos_callback);
    glfwSetCursorPosCallback(_window.get(), _internal::mouse_position_callback);
    glfwSetMouseButtonCallback(_window.get(), _internal::mouse_button_callback);
    glfwSetKeyCallback(_window.get(), _internal::key_callback);

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
    cleanObjects();
    _objects.shaders.clear();
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
    ptr->loadPreSetShaders();
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

void Window::cleanObjects() noexcept
{
    Context const context(_window.get());
    _objects.shaders.erase(
        _objects.shaders.cbegin(),
        _objects.shaders.find(static_cast<uint32_t>(Shaders::Preset::First))
    );
    _objects.renderers.clear();
    _objects.planes.clear();
    _objects.textures.clear();
    _objects.cameras.clear();
}

void Window::createShaders(uint32_t id) try
{
    if (id >= static_cast<uint32_t>(Shaders::Preset::First)) {
        LOG_METHOD_CTX_WRN("Given ID is in reserved values", id);
        return;
    }
    _objects.shaders.try_emplace(id);
    _objects.shaders.at(id).reset(new Shaders(weak_from_this()));
}
CATCH_AND_RETHROW_METHOD_EXC;

void Window::removeShaders(uint32_t id)
{
    if (id >= static_cast<uint32_t>(Shaders::Preset::First)) {
        LOG_METHOD_CTX_WRN("Given ID is in reserved values", id);
        return;
    }
    if (_objects.shaders.count(id) != 0) {
        _objects.shaders.erase(_objects.shaders.find(id));
    }
}

void Window::removeRenderer(uint32_t id)
{
    if (_objects.renderers.count(id) != 0) {
        _objects.renderers.erase(_objects.renderers.find(id));
    }
}

void Window::createModel(uint32_t id, Model::Type type) try
{
    switch (type) {
    case Model::Type::Plane:
        _objects.planes.try_emplace(id);
        _objects.planes.at(id).reset(new Plane(weak_from_this()));
        break;
    }
}
CATCH_AND_RETHROW_METHOD_EXC;

void Window::removeModel(uint32_t id, Model::Type type)
{
    switch (type) {
    case Model::Type::Plane:
        if (_objects.planes.count(id) != 0) {
            _objects.planes.erase(_objects.planes.find(id));
        }
        break;
    }
}

void Window::createTexture(uint32_t id) try
{
    _objects.textures.try_emplace(id);
    _objects.textures.at(id).reset(new Texture(weak_from_this()));
}
CATCH_AND_RETHROW_METHOD_EXC;

void Window::removeTexture(uint32_t id)
{
    if (_objects.textures.count(id) != 0) {
        _objects.textures.erase(_objects.textures.find(id));
    }
}

void Window::createCamera(uint32_t id) try
{
    _objects.cameras.try_emplace(id);
    _objects.cameras.at(id).reset(new Camera(weak_from_this()));
}
CATCH_AND_RETHROW_FUNC_EXC;

void Window::removeCamera(uint32_t id)
{
    if (_objects.cameras.count(id) != 0) {
        _objects.cameras.erase(_objects.cameras.find(id));
    }
}

    // --- Public methods ---

// Draws objects inside renderers on the back buffer.
void Window::drawObjects()
{
    if (!_is_iconified && isVisible()) {
        // Make context current for this scope
        Context const context(_window.get());
        // Render all active renderers
        for (auto it = _objects.renderers.cbegin(); it != _objects.renderers.cend(); ++it) {
            Renderer::Ptr const& renderer = it->second;
            if (!renderer || !renderer->isActive())
                continue;
            renderer->render();
        }
    }
}

// Renders back buffer, clears front buffer, polls events.
// Logs fps and/or longest_frame if specified in LOG structure.
void Window::printFrame() try
{
    using clock = std::chrono::steady_clock;
    // Render if visible
    if (!_is_iconified && isVisible()) {
        // Make context current for this scope
        Context const context(_window.get());

        // Limit fps if needed
        sleepUntil(_last_render_time + _min_frame_time);
        clock::time_point const now = clock::now();

        // Render back buffer
        glfwSwapBuffers(_window.get());
        // Update hovering status
        _updateHoveredModelIfNeeded(now);
        // Update last render time
        _last_render_time = now;
        // Update fps, log if needed
        if (_frame_timer.addFrame()) {
            // Log
            if (Log::GL::Window::query(Log::GL::Window::get().fps)) {
                char buff[256];
                sprintf_s(buff, "'%s' -> % 4lldfps, longest frame: % 4lldms",
                    _title.c_str(), _frame_timer.get(), _frame_timer.longestFrame());
                LOG_GL_MSG(buff);
            }
        }
        // Clear back buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    // Else, only update "render" time
    else {
        _last_render_time = clock::now();
    }
}
CATCH_AND_RETHROW_METHOD_EXC;

void Window::_updateHoveredModel()
{
    // Reset hovering
    _something_is_hovered = false;

    // If the cursor is disabled (Camera mode), then its relative position is at
    // the center of the window, which, on -1/+1 coordinates, is 0/0.
    float x = 0.f, y = 0.f;
    // Else find real coordinates
    if (glfwGetInputMode(_window.get(), GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
        // Retrieve cursor coordinates, ranging from 0 to width/height
        double x_offset, y_offset;
        glfwGetCursorPos(_window.get(), &x_offset, &y_offset);
        // Ensure cursor is inside the window
        double const w = static_cast<double>(_w), h = static_cast<double>(_h);
        if (x_offset < 0.0 || x_offset >= w || y_offset < 0.0 || y_offset >= h) {
            return;
        }
        // Normalize to -1/+1 coordinates
        x = static_cast<float>((x_offset / w * 2.0) - 1.0);
        y = static_cast<float>(((y_offset / h * 2.0) - 1.0) * -1.0);
    }

    double z = DBL_MAX;
    // Loop over each renderer (in reverse order) and find their nearest
    // models at mouse coordinates
    for (auto it = _objects.renderers.crbegin(); it != _objects.renderers.crend(); ++it) {
        // Retrieve Renderer's raw ptr needed for dynamic_cast
        Renderer* ptr = it->second.get();
        if (ptr == nullptr)
            continue;
        // Try to cast to Plane::Renderer, and find its nearest model
        Plane::Renderer* renderer = dynamic_cast<Plane::Renderer*>(ptr);
        if (renderer != nullptr && renderer->_findNearestModel(x, y)) {
            // If a model was found, update hover status
            _something_is_hovered = true;
            if (renderer->_hovered_z < z) {
                z = renderer->_hovered_z;
                _hovered_model_id = renderer->_hovered_id;
                _hovered_model_type = Model::Type::Plane;
            }
            // If the depth buffer was reset at least once by the renderer, previous
            // tested models will always be on top of all not-yet-tested models,
            // which means we must skip further tests.
            // This is why we're testing renderers in their reverse order.
            bool depth_buffer_was_reset = false;
            for (auto it = renderer->chunks.cbegin(); it != renderer->chunks.cend(); ++it) {
                if (it->reset_depth_before) {
                    depth_buffer_was_reset = true;
                    break;
                }
            }
            if (depth_buffer_was_reset)
                break;
        }
    }
    if (_something_is_hovered &&
        Log::GL::Window::query(Log::GL::Window::get().hovered_model)) {
        std::string model_type;
        switch (_hovered_model_type) {
        case Model::Type::Plane:
            model_type = "Plane";
            break;
        default:
            model_type = "Unknown";
        }
        char buff[256];
        sprintf_s(buff, "'%s' -> Hovered: %s #%u",
            _title.c_str(), model_type.c_str(), _hovered_model_id);
        LOG_GL_MSG(buff);
    }
    _hover_waiting_time = std::chrono::nanoseconds(0);
}

void Window::_updateHoveredModelIfNeeded(std::chrono::steady_clock::time_point const& now)
{
    static constexpr std::chrono::milliseconds threshold(50);

    // Bypass threshold if cursor just stopped moving
    double x, y;
    glfwGetCursorPos(_window.get(), &x, &y);
    if (x != _old_cursor_x || y != _old_cursor_y) {
        _cursor_is_moving = true;
        _old_cursor_x = x;
        _old_cursor_y = y;
    }
    else if (_cursor_is_moving && _hover_waiting_time >= std::chrono::milliseconds(10)) {
        _cursor_is_moving = false;
        _updateHoveredModel();
        return;
    }

    // Compute and add the time since last render to the waiting time.
    // Ensure that the user "going back in time" does not break this function.
    std::chrono::nanoseconds const delta_time = now - _last_render_time;
    if (delta_time < std::chrono::nanoseconds(0)) {
        _hover_waiting_time = threshold;
    }
    else {
        _hover_waiting_time += delta_time;
    }
    // If waited enough, retrieve mouse coordinates and check for model hovering
    // Return if mouse is outside window
    if (_hover_waiting_time >= threshold) {
        _updateHoveredModel();
    }
}

void Window::_callPassiveFunctions()
{
    for (auto it = _objects.planes.cbegin(); it != _objects.planes.cend(); ++it) {
        it->second->_callPassiveFunction(_window.get(), it->first);
    }
}

// Wether the user requested to close the window.
// NOTE: this simply is a call to glfwWindowShouldClose
bool Window::shouldClose() const noexcept
{
    return glfwWindowShouldClose(_window.get());
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
// -> screen_id of -1 (default) takes the monitor the window is on
void Window::setFullscreen(bool state, int screen_id)
{
    // Retrieve current fullscreen monitor (nullptr when windowed)
    GLFWmonitor* fullscreenMonitor = glfwGetWindowMonitor(_window.get());
    
    if (state) {
        // Ensure given ID is in range
        if (screen_id >= static_cast<int>(_monitors.size())) {
            LOG_METHOD_WRN("screen_id out of range.");
            return;
        }
        // Select monitor
        GLFWmonitor* monitor = screen_id < 0 ? _main_monitor : _monitors[screen_id];
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




void Context::_init(GLFWwindow* ptr)
{
    _given = ptr;
    _previous = glfwGetCurrentContext();
    _equal = _given == _previous;
    if (!_equal) {
        glfwMakeContextCurrent(_given);
        // Log
        if (Log::GL::Context::query(Log::GL::Context::get().set_context)) {
            char buff[256];
            sprintf_s(buff, "Context -> make current: %p (given)", ptr);
            LOG_GL_MSG(buff);
        }
    }
}

Context::Context(std::weak_ptr<Window> ptr)
{
    Window::Shared const window = ptr.lock();
    if (!window) {
        return;
    }
    _init(window->_window.get());
}

Context::Context(GLFWwindow* ptr)
{
    _init(ptr);
}

Context::~Context()
{
    if (!_equal) {
        glfwMakeContextCurrent(_previous);
        // Log
        if (Log::GL::Context::query(Log::GL::Context::get().set_context)) {
            char buff[256];
            sprintf_s(buff, "Context -> make current: %p (previous)", _previous);
            LOG_GL_MSG(buff);
        }
    }
}

SSS_GL_END;
