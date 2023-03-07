#include "SSS/GL/Window.hpp"

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

bool pollEverything() try
{
    using namespace std::chrono;
    static steady_clock::time_point last_poll = steady_clock::now();
    steady_clock::time_point const now = steady_clock::now();
    nanoseconds const time_since_last_poll = now - last_poll;

    bool ret = false;
    
    // Poll events
    glfwPollEvents();
    // Update every Text Area (this won't do anything if nothing is needed)
    TR::Area::updateAll();

    // Loop over each Window instance
    for (Window::Weak const& weak : Window::_instances) {
        Window::Shared window = weak.lock();
        if (!window) {
            continue;
        }
        if (window->isVisible()) {
            ret = true;
        }
        Context const context(window);

        // Process inputs
        processInputs(window->_key_queue, window->_key_inputs, window->_input_stack_time);
        processInputs(window->_click_queue, window->_click_inputs, window->_input_stack_time);

        Input left_click = window->getClickInputs()[GLFW_MOUSE_BUTTON_LEFT];
        window->_clicked_model.reset();
        if (left_click.is_pressed()) {
            window->_clicked_model = window->_hovered_model;
            window->_held_model = window->_hovered_model;
        }
        else if (left_click.is_released()) {
            window->_held_model.reset();
        }
    }
    
    // Loop over each Texture instance
    for (Texture::Weak const weak : Texture::_instances) {
        Texture::Shared const texture = weak.lock();
        if (!texture)
            continue;
        // Handle file loading threads, or text area threads
        if (texture->_type == Texture::Type::Raw) {
            // Skip if no thread is pending, or nothing was parsed
            auto& thread = texture->_loading_thread;
            if (thread.isPending() && !thread._frames.empty()) {
                // Move pixels from thread to texture instance, so that future
                // threads can run without affecting those pixels.
                texture->_frames = std::move(thread._frames);
                texture->_total_frames_time = thread._total_frames_time;
                // Update dimensions if needed, edit OpenGL texture
                texture->_raw_w = thread._w;
                texture->_raw_h = thread._h;
                texture->_updatePlanes();
                texture->_raw_texture.editSettings(thread._w, thread._h,
                    static_cast<uint32_t>(texture->_frames.size()));
                for (uint32_t i = 0; i < texture->_frames.size(); ++i) {
                    texture->_raw_texture.editPixels(texture->_frames[i].pixels.data(), i);
                }
                thread.setAsHandled();
            }
        }
        else if (texture->_type == Texture::Type::Text) {
            TR::Area* text_area = texture->getTextArea();
            // Skip if no Area is set or if no new pixels
            if (text_area && text_area->pixelsWereChanged()) {
                // Retrieve dimensions
                int new_w, new_h;
                text_area->pixelsGetDimensions(new_w, new_h);
                // Update dimensions if needed, edit OpenGL texture
                texture->_internalEdit(text_area->pixelsGet(), new_w, new_h);
            }
        }

    }

    // Loop over each Plane instance
    for (Plane::Weak const& weak : Plane::_instances) {
        Plane::Shared const plane = weak.lock();
        if (!plane) {
            continue;
        }
        
        if (plane->isPlaying()) {
            plane->_animation_duration += time_since_last_poll;
            plane->_updateTextureOffset();
        }
    }

    // Set all Area threds as handled, now that all textures are updated
    TR::Area::notifyAll();

    last_poll = now;
    return ret;
}
CATCH_AND_RETHROW_FUNC_EXC;

SSS_GL_END;