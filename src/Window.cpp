#include "SSS/GL/Window.hpp"
#include "SSS/GL/_internal/callbacks.hpp"

__SSS_GL_BEGIN

    // --- Static initializations ---

// Default log options
bool Window::LOG::constructor{ true };
bool Window::LOG::destructor{ true };
bool Window::LOG::glfw_init{ true };
bool Window::LOG::fps{ true };

// Window ptr
std::vector<std::weak_ptr<Window>> Window::_instances{};
// Connected monitors
std::vector<_internal::Monitor> Window::_monitors{};

    // --- Constructor & destructor ---

// Constructor, creates a window
Window::Window(Args const& args) try
{
    // Init GLFW
    if (_instances.empty()) {
        // Init GLFW
        glfwInit();
        if (LOG::glfw_init) {
            __LOG_OBJ_MSG("GLFW initialized.");
        }
        // Retrive monitors
        _internal::monitor_callback(nullptr, 0);
        glfwSetMonitorCallback(_internal::monitor_callback);
    }

    // Set main monitor
    if (args.monitor_id < 0 || static_cast<size_t>(args.monitor_id) >= _monitors.size()) {
        _setMainMonitor(_monitors[0]);
    }
    else {
        _setMainMonitor(_monitors[args.monitor_id]);
    }
    // Retrieve video size of monitor and adjust size parameters
    GLFWvidmode const* mode = glfwGetVideoMode(_main_monitor.ptr);
    _w = args.w < mode->width ? args.w : mode->width;
    _h = args.h < mode->height ? args.h : mode->height;

    // If in Debug mode, set the Debug hint
    if constexpr (DEBUGMODE) {
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    }
    // Create window
    _window.reset(glfwCreateWindow(
        _w,
        _h,
        args.title.c_str(),
        args.fullscreen ? _main_monitor.ptr : nullptr,
        nullptr
    ));
    // Throw if an error occured
    if (!_window.get()) {
        const char* msg;
        glfwGetError(&msg);
        throw_exc(msg);
    }
    // Center window on given monitor
    int x, y;
    glfwGetMonitorPos(_main_monitor.ptr, &x, &y);
    if (args.fullscreen) {
        // Retrieve actual width & height
        glfwGetWindowSize(_window.get(), &_w, &_h);
        _windowed_x = x + (mode->width - _w) / 2;
        _windowed_y = y + (mode->height - _h) / 2;
    }
    else {
        glfwSetWindowPos(_window.get(), x + (mode->width - _w) / 2, y + (mode->height - _h) / 2);
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

    // Set VSYNC to false by default
    setVSYNC(false);

    // Set projections
    _setProjections();

    // Set window callbacks, after everything else is set
    glfwSetWindowSizeCallback(_window.get(), _internal::window_resize_callback);
    glfwSetWindowPosCallback(_window.get(), _internal::window_pos_callback);
    glfwSetCursorPosCallback(_window.get(), _internal::mouse_position_callback);
    glfwSetMouseButtonCallback(_window.get(), _internal::mouse_button_callback);
    glfwSetKeyCallback(_window.get(), _internal::key_callback);

    if (LOG::constructor) {
        __LOG_CONSTRUCTOR
    }
}
__CATCH_AND_RETHROW_METHOD_EXC

// Destructor
Window::~Window()
{
    // Free all bound objects
    cleanObjects();
    // Remove weak ptr from instance vector
    cleanWeakPtrVector(_instances);
    // Terminate GLFW
    if (_instances.empty()) {
        glfwTerminate();
        if (LOG::glfw_init) {
            __LOG_OBJ_MSG("GLFW terminated.");
        }
    }
    if (LOG::destructor) {
        __LOG_DESTRUCTOR
    }
}

Window::Shared Window::create(Args const& args) try
{
    return (Shared)_instances.emplace_back(Shared(new Window(args)));
}
__CATCH_AND_RETHROW_FUNC_EXC

Window::Shared Window::get(GLFWwindow* ptr) try
{
    for (Weak const& weak : _instances) {
        Shared window = weak.lock();
        if (window && window->_window.get() == ptr) {
            return window;
        }
    }
    throw_exc(ERR_MSG::NOTHING_FOUND);
}
__CATCH_AND_RETHROW_FUNC_EXC

void Window::cleanObjects() noexcept
{
    Context const context(_window.get());
    _objects.models.classics.clear();
    _objects.models.planes.clear();
    _objects.models.buttons.clear();
    _objects.textures.clear();
    _objects.shaders.clear();
}

void Window::createModel(uint32_t id, ModelType type) try
{
    switch (type) {
    case ModelType::Classic:
        _objects.models.classics.try_emplace(id);
        _objects.models.classics.at(id).reset(new Model(weak_from_this()));
        break;
    case ModelType::Plane:
        _objects.models.planes.try_emplace(id);
        _objects.models.planes.at(id).reset(new Plane(weak_from_this()));
        break;
    case ModelType::Button:
        _objects.models.buttons.try_emplace(id);
        _objects.models.buttons.at(id).reset(new Button(weak_from_this()));
        break;
    }
}
__CATCH_AND_RETHROW_METHOD_EXC

void Window::removeModel(uint32_t id, ModelType type)
{
    switch (type) {
    case ModelType::Classic:
        if (_objects.models.classics.count(id) != 0) {
            _objects.models.classics.erase(_objects.models.classics.find(id));
        }
        break;
    case ModelType::Plane:
        if (_objects.models.planes.count(id) != 0) {
            _objects.models.planes.erase(_objects.models.planes.find(id));
        }
        break;
    case ModelType::Button:
        if (_objects.models.buttons.count(id) != 0) {
            _objects.models.buttons.erase(_objects.models.buttons.find(id));
        }
        break;
    }
}

void Window::createTexture(uint32_t id) try
{
    _objects.textures.try_emplace(id);
    _objects.textures.at(id).reset(new Texture(weak_from_this()));
}
__CATCH_AND_RETHROW_METHOD_EXC

void Window::removeTexture(uint32_t id)
{
    if (_objects.textures.count(id) != 0) {
        _objects.textures.erase(_objects.textures.find(id));
    }
}

void Window::pollTextureThreads() try
{
    // Loop over each Context instance
    for (Weak const& weak : _instances) {
        Shared window = weak.lock();
        if (!window) {
            continue;
        }
        Context const context(window);
        // Loop over each Texture2D instance
        auto const& map = window->_objects.textures;
        for (auto it = map.cbegin(); it != map.cend(); ++it) {
            Texture::Ptr const& tex = it->second;
            // If the loading thread is pending, edit the texture
            if (tex->_loading_thread.isPending()) {
                // Give the image to the OpenGL texture and notify all planes & buttons
                tex->_w = tex->_loading_thread._w;
                tex->_h = tex->_loading_thread._h;
                tex->_pixels = std::move(tex->_loading_thread._pixels);
                tex->_raw_texture.edit(&tex->_pixels[0], tex->_w, tex->_h);
                tex->_updatePlanesScaling();
                // Set thread as handled.
                tex->_loading_thread.setAsHandled();
            }
        }
    }
}
__CATCH_AND_RETHROW_FUNC_EXC

void Window::createShaders(uint32_t id, std::string const& vert_fp, std::string const& frag_fp) try
{
    _objects.shaders.try_emplace(id);
    _objects.shaders.at(id).reset(new Program(weak_from_this(), vert_fp, frag_fp));
}
__CATCH_AND_RETHROW_METHOD_EXC

void Window::removeShaders(uint32_t id)
{
    if (_objects.shaders.count(id) != 0) {
        _objects.shaders.erase(_objects.shaders.find(id));
    }
}

    // --- Public methods ---

// Renders a frame & polls events.
// Logs fps if specified in LOG structure.
void Window::render() try
{
    // Make context current for this scope
    Context const context(_window.get());
    // Render back buffer
    glfwSwapBuffers(_window.get());
    // Update fps, log if needed
    if (_fps_timer.addFrame()) {
        if (LOG::fps) {
            __LOG_OBJ_MSG(toString(_fps_timer.get()) + "fps");
        }
    }
    // Clear back buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Poll events
    glfwPollEvents();
}
__CATCH_AND_RETHROW_METHOD_EXC

// Wether the user requested to close the window.
// NOTE: this simply is a call to glfwWindowShouldClose
bool Window::shouldClose() const noexcept
{
    return glfwWindowShouldClose(_window.get());
}

// Enables or disables the VSYNC of the window
void Window::setVSYNC(bool state)
{
    // Make context current for this scope
    Context const context(_window.get());
    // Set VSYNC
    glfwSwapInterval(state);
}

// Enables or disables fullscreen mode on given screen
// -> screen_id of -1 (default) takes the monitor the window is on
void Window::setFullscreen(bool state, int screen_id)
{
    GLFWmonitor* currentMonitor = glfwGetWindowMonitor(_window.get());
    
    if (state) {
        // Ensure given ID is in range
        if (screen_id >= static_cast<int>(_monitors.size())) {
            __LOG_METHOD_WRN("screen_id out of range.");
            return;
        }
        // Select monitor
        _internal::Monitor const& monitor =
            screen_id < 0 ? _main_monitor : _monitors[screen_id];
        // Ensure window isn't already fullscreen on given ID
        if (currentMonitor == monitor.ptr) {
            __LOG_METHOD_WRN("window is already fullscreen on given screen");
            return;
        }
        // Store current size & pos, if currently in windowed mode
        if (currentMonitor == nullptr) {
            glfwGetWindowPos(_window.get(), &_windowed_x, &_windowed_y);
        }
        // Set window in fullscreen
        glfwSetWindowMonitor(_window.get(), monitor.ptr,
            0, 0, _w, _h, GLFW_DONT_CARE);
    }
    else {
        // Ensure the window isn't arealdy windowed
        if (currentMonitor == nullptr) {
            __LOG_METHOD_WRN("window is already windowed.");
            return;
        }
        // Set window in windowed mode with old values
        glfwSetWindowMonitor(_window.get(), nullptr,
            _windowed_x, _windowed_y, _w, _h, GLFW_DONT_CARE);
    }
}

void Window::setFOV(float radians)
{
    _fov = radians;
    _setProjections();
}

void Window::setProjectionRange(float z_near, float z_far)
{
    _z_near = z_near;
    _z_far = z_far;
    _setProjections();
}

void Window::_setMainMonitor(_internal::Monitor const& monitor)
{
    _main_monitor = monitor;
}

void Window::_setProjections() try
{
    float const ratio = getScreenRatio();
    // Set perspective projection
    _perspe_mat4 = glm::perspective(_fov, ratio, _z_near, _z_far);
    // Set ortho projection
    float x = ratio > 1.f ? ratio : 1.f;
    float y = ratio > 1.f ? 1.f : 1.f / ratio;
    _ortho_mat4 = glm::ortho(-x, x, -y, y, _z_near, _z_far);
}
__CATCH_AND_RETHROW_METHOD_EXC

Context::Context(std::weak_ptr<Window> ptr)
{
    if (ptr.expired()) {
        return;
    }
    Window::Shared const window = ptr.lock();
    _given = window->_window.get();
    _previous = glfwGetCurrentContext();
    _equal = _given == _previous;
    if (_equal) {
        return;
    }
    glfwMakeContextCurrent(_given);
}

Context::Context(GLFWwindow* ptr)
{
    _given = ptr;
    _previous = glfwGetCurrentContext();
    _equal = _given == _previous;
    if (_equal) {
        return;
    }
    glfwMakeContextCurrent(_given);
}

Context::~Context()
{
    if (_equal) {
        return;
    }
    glfwMakeContextCurrent(_previous);
}

__SSS_GL_END
