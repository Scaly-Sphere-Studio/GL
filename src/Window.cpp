#include "SSS/GL/Window.hpp"
#include "SSS/GL/_internal/callbacks.hpp"

__SSS_GL_BEGIN

    // --- Static initializations ---

// Default log options
bool Window::LOG::constructor{ true };
bool Window::LOG::destructor{ true };
bool Window::LOG::glfw_init{ true };
bool Window::LOG::fps{ true };
bool Window::LOG::dpi_update{ true };


// Window instances
std::map<GLFWwindow const*, Window::Weak> Window::_instances{};
// Connected monitors
std::vector<_internal::Monitor> Window::_monitors{};

    // --- Constructor & destructor ---

// Constructor, creates a window and makes its context current
Window::Window(int w, int h, std::string const& title) try
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
    // Retrieve video size of primary monitor
    GLFWvidmode const* mode = glfwGetVideoMode(_monitors[0].ptr);
    _w = w < mode->width ? w : mode->width;
    _h = h < mode->height ? h : mode->height;

    // Set projections
    _setProjections();

    // Hints
    //glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    // Create window
    _window.reset(glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr));
    // Throw if an error occured
    if (!_window.get()) {
        const char* msg;
        glfwGetError(&msg);
        throw_exc(msg);
    }
    // Make its context current
    use();

    // Init glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        SSS::throw_exc("Failed to initialize GLAD");
    }

    // Set viewport
    glViewport(0, 0, _w, _h);
    // Enable blending (transparency)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Set main monitor
    _setMainMonitor(_monitors[0]);
    // Set VSYNC to false by default
    setVSYNC(false);
    // Center window
    glfwSetWindowPos(_window.get(), (mode->width - _w) / 2, (mode->height - _h) / 2);
    // Set window callbacks
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
    for (auto it = _instances.cbegin(); it != _instances.cend();) {
        if (it->second.expired()) {
            it = _instances.erase(it);
        }
        else {
            ++it;
        }
    }
    if (_instances.empty()) {
        // Terminate GLFW
        glfwTerminate();
        if (LOG::glfw_init) {
            __LOG_OBJ_MSG("GLFW terminated.");
        }
    }
    if (LOG::destructor) {
        __LOG_DESTRUCTOR
    }
}

    // --- Instances storage ---

// Creates a window and returns a corresponding shared_ptr
Window::Shared Window::create(int w, int h, std::string const& title) try
{
    Shared window = Shared(new Window(w, h, title));
    _instances.emplace(window->_window.get(), window);
    return window;
}
__CATCH_AND_RETHROW_FUNC_EXC

// Returns an existing Window instance by its GLFWwindow pointer
Window::Shared Window::get(GLFWwindow const* ptr) try
{
    return _instances.at(ptr).lock();
}
__CATCH_AND_RETHROW_FUNC_EXC

Window::Shared Window::getMain() try
{
    return get(glfwGetCurrentContext());
}
__CATCH_AND_RETHROW_FUNC_EXC

    // --- Model methods ---

Model::Shared Window::createModel()
{
    // Use new instead of std::make_shared to access private constructor
    return Model::Shared(
        _models.emplace_back(Model::Shared(new Model())));
}

Plane::Shared Window::createPlane()
{
    return Plane::Shared(
        _planes.emplace_back(Plane::Shared(new Plane())));
}

Plane::Shared Window::createPlane(TextureBase::Shared texture)
{
    return Plane::Shared(
        _planes.emplace_back(Plane::Shared(new Plane(texture))));
}

Button::Shared Window::createButton()
{
    return Button::Shared(
        _buttons.emplace_back(Button::Shared(new Button())));
}

Button::Shared Window::createButton(TextureBase::Shared texture)
{
    return Button::Shared(
        _buttons.emplace_back(Button::Shared(new Button(texture, _window.get()))));
}

void Window::unloadModel(Model::Shared model)
{
    for (auto it = _models.cbegin(); it != _models.cend(); ++it) {
        if (*it == model) {
            _models.erase(it);
            break;
        }
    }
}

void Window::unloadPlane(Plane::Shared plane)
{
    for (auto it = _planes.cbegin(); it != _planes.cend(); ++it) {
        if (*it == plane) {
            _planes.erase(it);
            break;
        }
    }
}

void Window::unloadButton(Button::Shared button)
{
    for (auto it = _buttons.cbegin(); it != _buttons.cend(); ++it) {
        if (*it == button) {
            _buttons.erase(it);
            break;
        }
    }
}

void Window::unloadAllModels()
{
    _models.clear();
}

void Window::unloadAllPlanes()
{
    _planes.clear();
}

void Window::unloadAllButtons()
{
    _buttons.clear();
}

    // --- Public methods ---

// Renders a frame & polls events.
// Logs fps if specified in LOG structure.
void Window::render() try
{
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

// Make the OpenGL context this one.
void Window::use() const
{
    glfwMakeContextCurrent(_window.get());
}

// Wether the user requested to close the window.
// NOTE: this simply is a call to glfwWindowShouldClose
bool Window::shouldClose() const noexcept
{
    return glfwWindowShouldClose(_window.get());
}

// Enables or disables the VSYNC of the window
void Window::setVSYNC(bool state)
{
    // Ensure the current context is this window
    GLFWwindow* ptr = glfwGetCurrentContext();
    bool const isCurrent = ptr == _window.get();
    // Swap context if needed
    if (!isCurrent) {
        glfwMakeContextCurrent(_window.get());
    }
    // Set VSYNC
    glfwSwapInterval(state);
    // Re-swap context if needed
    if (!isCurrent) {
        glfwMakeContextCurrent(ptr);
    }
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

void Window::_setMainMonitor(_internal::Monitor const& monitor)
{
    _main_monitor = monitor;

    // Retrieve screen resolution (in pixels)
    const GLFWvidmode* mode = glfwGetVideoMode(monitor.ptr);

    // Horizontal DPI
    int const hdpi = std::lround(static_cast<float>(mode->width) / monitor.w);
    // Vertical DPI
    int const vdpi = std::lround(static_cast<float>(mode->height) / monitor.h);

    // Set TR's DPIs
    TR::Font::setDPI(FT_UInt(hdpi), FT_UInt(vdpi));
    if (LOG::dpi_update) {
        __LOG_MSG(context_msg("Text rendering DPI set", toString(hdpi)) + "x" + toString(vdpi));
    }
}

void Window::_setProjections()
{
    float const ratio = getScreenRatio();
    // Set perspective projection
    _perspe_mat4 = glm::perspective(_fov, ratio, _z_near, _z_far);
    // Set ortho projection
    float x = ratio > 1.f ? ratio : 1.f;
    float y = ratio > 1.f ? 1.f : 1.f / ratio;
    _ortho_mat4 = glm::ortho(-x, x, -y, y, _z_near, _z_far);
}

void Window::_textureWasEdited(TextureBase::Shared texture)
{
    for (auto it = _instances.cbegin(); it != _instances.cend(); ++it) {
        if (it->second.expired()) {
            continue;
        }
        Shared window = it->second.lock();
        for (Plane::Shared& plane : window->_planes) {
            if (plane->_texture == texture)
                plane->_updateTexScaling();
        }
        for (Button::Shared& button : window->_buttons) {
            if (button->_texture == texture)
                button->_updateTexScaling();
        }
    }
}

__SSS_GL_END
