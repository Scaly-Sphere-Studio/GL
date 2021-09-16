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

    // Set window callbacks, after everything else is set
    glfwSetWindowIconifyCallback(_window.get(), _internal::window_iconify_callback);
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
    _objects.models.clear();
    _objects.planes.clear();
    _objects.textures.clear();
    _objects.cameras.clear();
    _objects.shaders.clear();
    _objects.renderers.clear();
}

void Window::createModel(uint32_t id, ModelType type) try
{
    switch (type) {
    case ModelType::Classic:
        _objects.models.try_emplace(id);
        _objects.models.at(id).reset(new Model(weak_from_this()));
        break;
    case ModelType::Plane:
        _objects.planes.try_emplace(id);
        _objects.planes.at(id).reset(new Plane(weak_from_this()));
        break;
    }
}
__CATCH_AND_RETHROW_METHOD_EXC

void Window::removeModel(uint32_t id, ModelType type)
{
    switch (type) {
    case ModelType::Classic:
        if (_objects.models.count(id) != 0) {
            _objects.models.erase(_objects.models.find(id));
        }
        break;
    case ModelType::Plane:
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
        // Loop over each Texture instance
        auto const& textures = window->_objects.textures;
        for (auto it = textures.cbegin(); it != textures.cend(); ++it) {
            Texture::Ptr const& tex = it->second;
            // Handle file loading threads, or text area threads
            if (tex->_type == Texture::Type::Raw) {
                // Skip if no thread is pending
                auto& thread = tex->_loading_thread;
                if (!thread.isPending()) {
                    continue;
                }
                // Move pixel vector from thread to texture instance, so that future
                // threads can run without affecting those pixels.
                tex->_pixels = std::move(thread._pixels);
                // Check if dimensions changed
                if (tex->_raw_w != thread._w || tex->_raw_h != thread._h) {
                    tex->_raw_w = thread._w;
                    tex->_raw_h = thread._h;
                    tex->_updatePlanesScaling();
                }
                // Edit OpenGL texture & set thread as handled
                tex->_raw_texture.edit(&tex->_pixels[0], tex->_raw_w, tex->_raw_h);
                thread.setAsHandled();
            }
            else if (tex->_type == Texture::Type::Text) {
                // Update the TextArea (this won't do anything if nothing is needed)
                tex->_text_area->update();
                // Skip if no thread is pending
                if (!tex->_text_area->changesPending()) {
                    continue;
                }
                // Check if dimensions changed
                int const w = tex->_text_w, h = tex->_text_h;
                tex->_text_area->getDimensions(tex->_text_w, tex->_text_h);
                if (w != tex->_text_w || h != tex->_text_h) {
                    tex->_updatePlanesScaling();
                }
                // Edit OpenGL texture & set thread as handled
                tex->_raw_texture.edit(tex->_text_area->getPixels(), tex->_text_w, tex->_text_h);
                tex->_text_area->changesHandled();
            }
        }
    }
}
__CATCH_AND_RETHROW_FUNC_EXC

void Window::createCamera(uint32_t id) try
{
    _objects.cameras.try_emplace(id);
    _objects.cameras.at(id).reset(new Camera(weak_from_this()));
}
__CATCH_AND_RETHROW_FUNC_EXC

void Window::removeCamera(uint32_t id)
{
    if (_objects.cameras.count(id) != 0) {
        _objects.cameras.erase(_objects.cameras.find(id));
    }
}

void Window::createShaders(uint32_t id) try
{
    _objects.shaders.try_emplace(id);
    _objects.shaders.at(id).reset(new Shaders(weak_from_this()));
}
__CATCH_AND_RETHROW_METHOD_EXC

void Window::removeShaders(uint32_t id)
{
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

    // --- Public methods ---

// Renders a frame & polls events.
// Logs fps if specified in LOG structure.
void Window::render() try
{
    std::chrono::steady_clock::time_point const now = std::chrono::steady_clock::now();
    if (!_is_iconified) {
        // Make context current for this scope
        Context const context(_window.get());
        // Update hovering status
        _updateHoveredModelIfNeeded(now);
        // Render all active renderers
        for (auto it = _objects.renderers.cbegin(); it != _objects.renderers.cend(); ++it) {
            Renderer::Ptr const& renderer = it->second;
            if (!renderer || !renderer->isActive())
                continue;
            renderer->render();
        }
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
    }
    // Poll events & update timepoint
    glfwPollEvents();
    _last_render_time = now;
}
__CATCH_AND_RETHROW_METHOD_EXC

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
    // Loop over each renderer and find their "nearest" models at mouse coordinates
    for (auto it = _objects.renderers.crbegin(); it != _objects.renderers.crend(); ++it) {
        // Retrieve PlaneRenderer
        Renderer::Ptr const& renderer = it->second;
        if (!renderer)
            continue;
        PlaneRenderer* ptr = dynamic_cast<PlaneRenderer*>(renderer.get());
        // Find nearest model
        if (ptr != nullptr && ptr->_findNearestModel(x, y)) {
            // If a model was found, update hover status
            _something_is_hovered = true;
            if (ptr->_hovered_z < z) {
                z = ptr->_hovered_z;
                _hovered_model_id = ptr->_hovered_plane;
                _hovered_model_type = ModelType::Plane;
            }
            // If the depth buffer was reset at least once by the renderer, previous
            // tested models will always be on top of all not-yet-tested models,
            // which means we must skip further tests. This is why we're testing
            // renderers in their reverse order.
            bool depth_buffer_was_reset = false;
            for (auto it = ptr->cbegin(); it != ptr->cend(); ++it) {
                if (it->second.reset_depth_before) {
                    depth_buffer_was_reset = true;
                    break;
                }
            }
            if (depth_buffer_was_reset)
                break;
        }
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

void Window::_setMainMonitor(_internal::Monitor const& monitor)
{
    _main_monitor = monitor;
}

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
