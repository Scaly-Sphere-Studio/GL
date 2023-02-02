#include "SSS/GL/Window.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#pragma warning(suppress : 4996)
#include <stb_image_write.h>
#include <filesystem>

SSS_GL_BEGIN;

// Draws objects inside renderers on the back buffer.
void Window::drawObjects()
{
    if (!isIconified() && isVisible()) {
        // Make context current for this scope
        Context const context(_window.get());
        // Render all active renderers
        for (auto const& renderer : _renderers) {
            if (!renderer || !renderer->isActive())
                return;
            renderer->render();
        }
    }
}

void Window::_updateHoveredModel()
{
    // Reset hovering
    _hovered_type = HoveredType::None;
    double z = DBL_MAX;

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

    // Loop over each renderer (in reverse order) and find their nearest
    // models at mouse coordinates
    for (auto const& renderer : _renderers) {
        if (!renderer || !renderer->isActive())
            continue;
        RendererBase* ptr = renderer.get();
        // Try to cast to Plane::Renderer, and find its nearest model
        PlaneRenderer* renderer = dynamic_cast<PlaneRenderer*>(ptr);
        if (renderer != nullptr && renderer->_findNearestModel(x, y)) {
            // If a model was found, update hover status
            if (renderer->_hovered_z < z) {
                z = renderer->_hovered_z;
                _hovered_type = HoveredType::Plane;
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
    if (_hovered_type != HoveredType::None
        && Log::GL::Window::query(Log::GL::Window::get().hovered_model)) {
        std::string model_type;
        switch (_hovered_type) {
        case HoveredType::Plane:
            model_type = "Plane";
            break;
        default:
            model_type = "Unknown";
        }
        char buff[256];
        sprintf_s(buff, "'%s' -> Hovered: %s #%u",
            _title.c_str(), model_type.c_str(), _hovered_id);
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

void Window::AsyncScreenshot::_asyncFunction(int w, int h, std::vector<uint8_t> pixels,
    std::string filename)
{
    // Reverse image
    for (int line = 0; line != h / 2; ++line) {
        std::swap_ranges(
            pixels.begin() + 3 * w * line,          // Source (start)
            pixels.begin() + 3 * w * (line + 1),    // Source (end)
            pixels.begin() + 3 * w * (h - line - 1) // Destination
        );
    }

    stbi_write_png(filename.c_str(), w, h, 3, &pixels[0], 0);
}

void Window::_saveScreenshot()
{
    if (!take_screenshot) {
        return;
    }

    // Remove finished operations
    _screenshots.remove_if([](AsyncPair const& pair) { return !pair.first.isRunning(); });
    
    // Read frame buffer
    std::vector<uint8_t> pixels(3 * _w * _h);
    glReadPixels(0, 0, _w, _h, GL_RGB, GL_UNSIGNED_BYTE, &pixels[0]);
    
    // Create screenshot directory if needed
    std::string const dir = SSS::PWD + "screenshots/";
    std::filesystem::create_directory(dir);

    // Format time
    std::string time = std::format("{:%Y%m%d%H%M%S}", std::chrono::system_clock::now());
    size_t const index = time.find('.');
    if (index < time.size()) {
        time.resize(index);
    }

    // Ensure filename is unique
    std::string filename;
    for (int i = 1;; ++i) {
        filename = dir + time + "_" + std::to_string(i) + ".png";
        bool loop = false;
        for (AsyncPair const& pair : _screenshots) {
            if (pair.second == filename) {
                loop = true;
                break;
            }
        }
        if (pathIsFile(filename)) {
            loop = true;
        }
        if (!loop) break;
    }

    // Run async file writing
    AsyncPair& pair = _screenshots.emplace_back();
    pair.first.run(_w, _h, pixels, filename);
    pair.second = filename;

    // Reset state
    take_screenshot = false;
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
        // Take screenshot if required
        _saveScreenshot();
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

SSS_GL_END;