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
                // Update dimensions if needed, edit OpenGL texture
                tex->_internalEdit(&tex->_pixels[0], thread._w, thread._h);
                thread.setAsHandled();
            }
            else if (tex->_type == Texture::Type::Text) {
                TR::Area* text_area = tex->getTextArea();
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
                tex->_internalEdit(text_area->pixelsGet(), new_w, new_h);
            }
        }
    }
    
    // Set all Area threds as handled, now that all textures are updated
    TR::Area::notifyAll();

    return ret;
}
CATCH_AND_RETHROW_FUNC_EXC;

SSS_GL_END;