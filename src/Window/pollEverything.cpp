#include "GL/Window.hpp"
#include "GL/Objects/Texture.hpp"
#include "GL/Objects/Models/Plane.hpp"

SSS_GL_BEGIN;

template<size_t size>
static void processInputs(  std::queue<std::pair<int, bool>>& queue,
                            std::array<Input, size>& inputs,
                            std::chrono::milliseconds const& input_stack_time)
{
    // Mark previously pressed inputs as handled so that is_pressed() returns false
    for (Input& input : inputs) {
        input.handled();
    }
    // Process input queue
    for (; !queue.empty(); queue.pop()) {
        auto const& queued_input = queue.front();
        Input& input = inputs[queued_input.first];
        if (queued_input.second) {
            input.increment(input_stack_time);
        }
        else {
            input.reset();
        }
    }
}

void Window::_poll()
{
    // Process inputs

    bool emitIptKey = !_key_queue.empty();
    bool emitMse    = !_click_queue.empty();

    processInputs(_key_queue, _key_inputs, _input_stack_time);
    if (emitIptKey) {
        EMIT_EVENT("SSS_WINDOW_KEY_INPUT");
        LOG_MSG("KEY INPUT");
    }

    processInputs(_click_queue, _click_inputs, _input_stack_time);
    if (emitMse) {
        EMIT_EVENT("SSS_WINDOW_MOUSE_INPUT");
        LOG_MSG("MOUSE INPUT");
    }
    // Mouse position (no queue because it's only x & y)
    if (!_block_inputs) {
        double x, y;
        glfwGetCursorPos(getGLFWwindow(), &x, &y);
        _old_cursor_x = _cursor_x;
        _old_cursor_y = _cursor_y;
        _cursor_x = static_cast<int>(x);
        _cursor_y = static_cast<int>(y);
        _cursor_diff_x = _cursor_x - _old_cursor_x;
        _cursor_diff_y = _old_cursor_y - _cursor_y; // reverse y coords
    }

    Input const left_click = getClickInputs()[GLFW_MOUSE_BUTTON_LEFT];
    _clicked.model.reset();
    _clicked.camera.reset();
    if (left_click.is_pressed()) {
        _clicked.model = _hovered.model;
        _clicked.camera = _hovered.camera;
        _held.model = _hovered.model;
        _held.camera = _hovered.camera;
    }
    else if (left_click.is_released()) {
        _held.model.reset();
        _held.camera.reset();
    }
}

void pollEverything() try
{
    using namespace std::chrono;
    static steady_clock::time_point last_poll = steady_clock::now();
    steady_clock::time_point const now = steady_clock::now();
    nanoseconds const time_since_last_poll = now - last_poll;

    // Poll events
    glfwPollEvents();

    // Poll threads
    pollAsync();

    // Update every Text Area (this won't do anything if nothing is needed)
    TR::Area::updateAll();

    // Poll all windows
    Window::_main->_poll();
    for (auto& [ptr, win] : Window::_main._subs)
        win->_poll();

    // Loop over each Plane instance
    for (auto& ref : PlaneBase::_instances) {
        PlaneBase& plane = ref.get();
        if (plane.isPlaying()) {
            plane._animation_duration += time_since_last_poll;
            plane._updateTextureOffset();
        }
    }

    last_poll = now;
}
CATCH_AND_RETHROW_FUNC_EXC;

SSS_GL_END;