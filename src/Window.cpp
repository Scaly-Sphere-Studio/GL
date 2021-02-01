#include "SSS/GL/Window.hpp"

__SSS_GL_BEGIN

    // --- Static initializations ---

// Default log options
bool Window::LOG::constructor{ true };
bool Window::LOG::destructor{ true };
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
    // Retrieve video size of primary monitor
    GLFWvidmode const* mode = glfwGetVideoMode(_monitors[0].ptr);
    _w = w < mode->width ? w : mode->width;
    _h = h < mode->height ? h : mode->height;

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
    glfwMakeContextCurrent(_window.get());

    // Initialize GLEW
    GLenum const glew_ret = glewInit();
    // Throw if an error occured
    if (glew_ret != GLEW_OK) {
        char const* msg = reinterpret_cast<char const*>(glewGetErrorString(glew_ret));
        throw_exc(msg);
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

    // --- Public methods ---

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

// Renders a frame & polls events.
// Logs fps if specified in LOG structure.
void Window::render()
{
    // Render back buffer
    glfwSwapBuffers(_window.get());
    // Update fps, log if needed
    if (fps_timer.addFrame()) {
        if (LOG::fps) {
            __LOG_MSG(toString(fps_timer.get()) + "fps");
        }
    }
    // Clear back buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Poll events
    glfwPollEvents();
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

__SSS_GL_END