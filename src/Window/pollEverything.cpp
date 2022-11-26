#include "SSS/GL/Window.hpp"
#include "SSS/Text-Rendering.hpp"
#include "SSS/GL/Objects/Texture.hpp"

SSS_GL_BEGIN;

bool pollEverything() try
{
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
        // Call all passive functions
        window->_callPassiveFunctions();
        // Loop over each Texture instance
        for (auto const& pair : window->_textures) {
            if (!pair.second)
                continue;
            Texture& texture = *pair.second;
            // Handle file loading threads, or text area threads
            if (texture._type == Texture::Type::Raw) {
                // Skip if no thread is pending, or nothing was parsed
                auto& thread = texture._loading_thread;
                if (thread.isPending() && !thread._frames.empty()) {
                    // Move pixels from thread to texture instance, so that future
                    // threads can run without affecting those pixels.
                    texture._frame_id = 0;
                    texture._frames = std::move(thread._frames);
                    // Update dimensions if needed, edit OpenGL texture
                    texture._raw_w = thread._w;
                    texture._raw_h = thread._h;
                    texture._updatePlanesScaling();
                    texture._raw_texture.editSettings(thread._w, thread._h, texture._frames.size());
                    for (size_t i = 0; i < texture._frames.size(); ++i) {
                        texture._raw_texture.editPixels(texture._frames[i].pixels.data(), i);
                    }
                    thread.setAsHandled();
                }
            }
            else if (texture._type == Texture::Type::Text) {
                TR::Area* text_area = texture.getTextArea();
                // Skip if no Area is set or if no new pixels
                if (text_area && text_area->pixelsWereChanged()) {
                    // Retrieve dimensions
                    int new_w, new_h;
                    text_area->pixelsGetDimensions(new_w, new_h);
                    // Update dimensions if needed, edit OpenGL texture
                    texture._internalEdit(text_area->pixelsGet(), new_w, new_h);
                }
            }
        }
    }
    
    // Set all Area threds as handled, now that all textures are updated
    TR::Area::notifyAll();

    return ret;
}
CATCH_AND_RETHROW_FUNC_EXC;

SSS_GL_END;