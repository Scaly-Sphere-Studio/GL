#include "SSS/GL/Texture.hpp"
#include "SSS/GL/Window.hpp"

// Init STB
#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include <stb_image.h>

__SSS_GL_BEGIN

// Init statics
bool Texture::LOG::constructor{ false };
bool Texture::LOG::destructor{ false };

Texture::Texture(std::weak_ptr<Window> window) try
    : _internal::WindowObject(window),
    _raw_texture(window, GL_TEXTURE_2D)
{
    Context const context(_window);
    _raw_texture.bind();
    _raw_texture.parameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    _raw_texture.parameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    _raw_texture.parameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    _raw_texture.parameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    _text_area = TR::TextArea::create(0, 0);

    if (LOG::constructor) {
        __LOG_CONSTRUCTOR
    }
}
__CATCH_AND_RETHROW_METHOD_EXC

Texture::~Texture()
{
    if (LOG::destructor) {
        __LOG_DESTRUCTOR
    }
}

void Texture::edit(void const* pixels, int width, int height) try
{
    // Update size info
    _w = width;
    _h = height;
    // Give the image to the OpenGL texture
    _raw_texture.edit(pixels, _w, _h);
    // Clear previous pixel storage
    _pixels.clear();

    _updatePlanesScaling();
}
__CATCH_AND_RETHROW_METHOD_EXC

void Texture::useFile(std::string filepath)
{
    _loading_thread.run(filepath);
}

void Texture::bind()
{
    Context const context(_window);
    if (_type == Type::Text) {
        _text_area->update();
        if (_text_area->changesPending()) {
            int const w = _w, h = _h;
            _text_area->getDimensions(_w, _h);
            if (w != _w || h != _h) {
                _updatePlanesScaling();
            }
            _raw_texture.edit(_text_area->getPixels(), _w, _h);
            _text_area->changesHandled();
        }
    }
    _raw_texture.bind();
}

void Texture::_updatePlanesScaling()
{
    Window::Shared const window = _window.lock();
    if (!window) {
        return;
    }
    Window::Objects const& objects = window->getObjects();
    // Retrieve texture ID
    uint32_t id = 0;
    for (auto it = objects.textures.cbegin(); it != objects.textures.cend(); ++it) {
        if (it->second.get() == this) {
            id = it->first;
            break;
        }
    }
    // Update texture scaling of all planes & buttons matching this texture
    for (auto it = objects.models.planes.cbegin(); it != objects.models.planes.cend(); ++it) {
        Plane::Ptr const& plane = it->second;
        if (plane->_use_texture && plane->_texture_id == id) {
            plane->_updateTexScaling();
        }
    }
    for (auto it = objects.models.buttons.cbegin(); it != objects.models.buttons.cend(); ++it) {
        Button::Ptr const& button = it->second;
        if (button->_use_texture && button->_texture_id == id) {
            button->_updateTexScaling();
        }
    }
}

void Texture::_LoadingThread::_function(std::string filepath)
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