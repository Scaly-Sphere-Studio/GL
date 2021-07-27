#include "SSS/GL/Texture2D.hpp"
#include "SSS/GL/Window.hpp"

// Init STB
#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include <stb_image.h>

__SSS_GL_BEGIN

// Init statics
bool Texture2D::LOG::constructor{ false };
bool Texture2D::LOG::destructor{ false };

Texture2D::Texture2D(std::weak_ptr<Window> window) try
    : TextureBase(window, GL_TEXTURE_2D)
{
    Context const context(_window);
    _raw_texture.bind();
    _raw_texture.parameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    _raw_texture.parameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    _raw_texture.parameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    _raw_texture.parameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    if (LOG::constructor) {
        __LOG_CONSTRUCTOR
    }
}
__CATCH_AND_RETHROW_METHOD_EXC

Texture2D::~Texture2D()
{
    if (LOG::destructor) {
        __LOG_DESTRUCTOR
    }
}

void Texture2D::useFile(std::string filepath)
{
    _loading_thread.run(filepath);
}

void Texture2D::edit(void const* pixels, int width, int height) try
{
    // Update size info
    _tex_w = width;
    _tex_h = height;
    // Give the image to the OpenGL texture
    _raw_texture.edit(pixels, _tex_w, _tex_h);
    // Clear previous pixel storage
    _raw_pixels.clear();

    _updatePlanesScaling();
}
__CATCH_AND_RETHROW_METHOD_EXC

void Texture2D::_updatePlanesScaling()
{
    if (_window.expired()) {
        return;
    }
    Window::Objects const& objects = _window.lock()->getObjects();
    // Retrieve texture ID
    uint32_t id = 0;
    for (auto it = objects.textures.classics.cbegin(); it != objects.textures.classics.cend(); ++it) {
        if (it->second.get() == this) {
            id = it->first;
            break;
        }
    }
    // Update texture scaling of all planes & buttons matching this texture
    for (auto it = objects.models.planes.cbegin(); it != objects.models.planes.cend(); ++it) {
        Plane::Ptr const& plane = it->second;
        if (plane->_texture_type == TextureType::Classic && plane->_texture_id == id) {
            plane->_updateTexScaling();
        }
    }
    for (auto it = objects.models.buttons.cbegin(); it != objects.models.buttons.cend(); ++it) {
        Button::Ptr const& button = it->second;
        if (button->_texture_type == TextureType::Classic && button->_texture_id == id) {
            button->_updateTexScaling();
            button->_updateWinScaling();
        }
    }
}

void Texture2D::_LoadingThread::_function(std::string filepath)
{
    // Load image
    SSS::C_Ptr <uint32_t, void(*)(void*), stbi_image_free>
        raw_pixels((uint32_t*)(stbi_load(
            filepath.c_str(),   // Filepath to picture
            &_w,                // Width, to query
            &_h,                // Height, to query
            nullptr,            // Byte composition, to query if not requested
            4                   // Byte composition, to request
        )));
    // Throw if error
    if (raw_pixels == nullptr) {
        SSS::throw_exc(stbi_failure_reason());
    }
    // Fill vector
    if (_is_canceled) return;
    _pixels = RGBA32::Pixels(raw_pixels.get(), raw_pixels.get() + (_w * _h));
}

__SSS_GL_END