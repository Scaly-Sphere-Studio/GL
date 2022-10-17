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
        for (auto it = window->_textures.cbegin(); it != window->_textures.cend(); ++it) {
            if (!it->second)
                continue;
            Texture& texture = *it->second;
            // Handle file loading threads, or text area threads
            if (texture._type == Texture::Type::Raw) {
                // Skip if no thread is pending
                auto& thread = texture._loading_thread;
                if (!thread.isPending()) {
                    continue;
                }
                // Move pixel vector from thread to texture instance, so that future
                // threads can run without affecting those pixels.
                texture._pixels = std::move(thread._pixels);
                // Update dimensions if needed, edit OpenGL texture
                texture._internalEdit(&texture._pixels[0], thread._w, thread._h);
                thread.setAsHandled();
            }
            else if (texture._type == Texture::Type::Text) {
                TR::Area* text_area = texture.getTextArea();
                // Skip if no Area is set
                if (!text_area) {
                    continue;
                }
                // Skip if no new pixels
                if (!text_area->pixelsWereChanged()) {
                    continue;
                }
                // Retrieve dimensions
                int new_w, new_h;
                text_area->pixelsGetDimensions(new_w, new_h);
                // Update dimensions if needed, edit OpenGL texture
                texture._internalEdit(text_area->pixelsGet(), new_w, new_h);
            }
        }
    }
    
    // Set all Area threds as handled, now that all textures are updated
    TR::Area::notifyAll();

    return ret;
}
CATCH_AND_RETHROW_FUNC_EXC;

SSS_GL_END;