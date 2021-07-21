#include "SSS/GL/Window.hpp"
#include "SSS/GL/Context.hpp"
#include "SSS/GL/_internal/callbacks.hpp"

__SSS_GL_BEGIN

    // --- Static initializations ---

// Default log options
bool Window::LOG::constructor{ true };
bool Window::LOG::destructor{ true };
bool Window::LOG::fps{ true };

// Connected monitors
std::vector<_internal::Monitor> Window::_monitors{};

    // --- Constructor & destructor ---

// Constructor, creates a window and makes its context current
Window::Window(std::shared_ptr<Context> context, GLFWwindow* win_ptr, Args const& args) try
    : _internal::ContextObject(context)
{
    ContextManager const context_manager(_context.lock());

    // Set main monitor
    _setMainMonitor(_monitors[args.monitor_id]);
    // Retrieve video size of monitor and adjust size parameters
    GLFWvidmode const* mode = glfwGetVideoMode(_main_monitor.ptr);
    _w = args.w < mode->width ? args.w : mode->width;
    _h = args.h < mode->height ? args.h : mode->height;

    // Create window
    _window.reset(glfwCreateWindow(
        _w,
        _h,
        args.title.c_str(),
        args.fullscreen ? _main_monitor.ptr : nullptr,
        win_ptr
    ));

    // Throw if an error occured
    if (!_window.get()) {
        const char* msg;
        glfwGetError(&msg);
        throw_exc(msg);
    }

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
    // Center window
    // TODO: fix monitor pos
    glfwSetWindowPos(_window.get(), (mode->width - _w) / 2, (mode->height - _h) / 2);
    // Set window callbacks
    glfwSetWindowSizeCallback(_window.get(), _internal::window_resize_callback);
    glfwSetWindowPosCallback(_window.get(), _internal::window_pos_callback);
    glfwSetCursorPosCallback(_window.get(), _internal::mouse_position_callback);
    glfwSetMouseButtonCallback(_window.get(), _internal::mouse_button_callback);
    glfwSetKeyCallback(_window.get(), _internal::key_callback);

    // Set projections
    _setProjections();

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

    // --- Public methods ---

// Renders a frame & polls events.
// Logs fps if specified in LOG structure.
void Window::render() try
{
    ContextManager const context_manager(_context.lock());
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
    ContextManager const context_manager(_context.lock());
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

inline void Window::setFOV(float radians)
{
    _fov = radians;
    _setProjections();
}

inline void Window::setProjectionRange(float z_near, float z_far)
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

__SSS_GL_END
