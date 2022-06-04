#include "SSS/GL/Window.hpp"

SSS_GL_BEGIN;

bool pollEverything() try
{
    bool ret = false;
    
    // Print all frames
    for (Window::Weak const& weak : Window::_instances) {
        Window::Shared window = weak.lock();
        if (!window) {
            continue;
        }
        window->printFrame();
    }

    // Poll events
    glfwPollEvents();

    // Retrieve all Text Areas
    TR::Area::Map const& text_areas = TR::Area::getMap();
    // Update every Text Area (this won't do anything if nothing is needed)
    for (auto it = text_areas.cbegin(); it != text_areas.cend(); ++it) {
        it->second->update();
    }
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
                TR::Area::Ptr const& text_area = tex->getTextArea();
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
                text_area->getDimensions(new_w, new_h);
                // Update dimensions if needed, edit OpenGL texture
                tex->_internalEdit(text_area->pixelsGet(), new_w, new_h);
            }
        }
    }
    // Set all Area threds as handled, now that all textures are updated
    for (auto it = text_areas.cbegin(); it != text_areas.cend(); ++it) {
        if (it->second->pixelsWereChanged()) {
            it->second->pixelsAreRetrieved();
        }
    }

    return ret;
}
CATCH_AND_RETHROW_FUNC_EXC;

SSS_GL_END;