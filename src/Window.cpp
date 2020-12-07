#include "SSS/GLFW/Window.hpp"

__SSS_GLFW_BEGIN

    // --- Static initializations ---

// Default log options
bool Window::LOG::constructor{ true };
bool Window::LOG::destructor{ true };
bool Window::LOG::fps{ true };

// Static members
std::map<GLFWwindow const*, Window::Weak> Window::_instances{};

    // --- Callbacks ---

__INTERNAL_BEGIN

// Resizes the internal width and height of correspondig Window instance
void window_resize_callback(GLFWwindow* ptr, int w, int h)
{
    Window::Shared window = Window::get(ptr);
    window->_w = w;
    window->_h = h;
}

__INTERNAL_END

    // --- Constructor & destructor ---

// Constructor, creates a window and makes its context current
Window::Window(int w, int h, std::string const& title) try
    : _w(w), _h(h)
{
    // Hints
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

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

    // Set window callbacks
    setCallback(glfwSetWindowSizeCallback, _internal::window_resize_callback);
    // Set VSYNC to false by default
    setVSYNC(false);

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
void Window::setFullscreen(bool state, int screen_id)
{
    int monitor_count;
    GLFWmonitor** monitors = glfwGetMonitors(&monitor_count);
    GLFWmonitor* currentMonitor = glfwGetWindowMonitor(_window.get());
    
    if (state) {
        // Ensure given ID is in range
        if (screen_id >= monitor_count) {
            __LOG_METHOD_WRN("screen_id out of range.");
            return;
        }
        // Ensure window isn't already fullscreen on given ID
        if (currentMonitor == monitors[screen_id]) {
            __LOG_METHOD_WRN("window is already fullscreen on given screen");
            return;
        }
        // Store current size & pos, if currently in windowed mode
        if (currentMonitor == nullptr) {
            _windowed_w = _w;
            _windowed_h = _h;
            glfwGetWindowPos(_window.get(), &_windowed_x, &_windowed_y);
        }
        // Get monitor's size
        GLFWvidmode const* mode = glfwGetVideoMode(monitors[screen_id]);
        // Set window in fullscreen
        glfwSetWindowMonitor(_window.get(), monitors[screen_id],
            0, 0, mode->width, mode->height, GLFW_DONT_CARE);
    }
    else {
        // Ensure the window isn't arealdy windowed
        if (currentMonitor == nullptr) {
            __LOG_METHOD_WRN("window is already windowed.");
            return;
        }
        // Set window in windowed mode with old values
        glfwSetWindowMonitor(_window.get(), nullptr,
            _windowed_x, _windowed_y, _windowed_w, _windowed_h, GLFW_DONT_CARE);
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
    glClear(GL_COLOR_BUFFER_BIT);
    // Poll events
    glfwPollEvents();
}

__SSS_GLFW_END