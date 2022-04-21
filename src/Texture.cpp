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
    _type = Type::Raw;
    // Update size info
    _raw_w = width;
    _raw_h = height;
    // Give the image to the OpenGL texture
    _raw_texture.edit(pixels, _raw_w, _raw_h);
    // Replace previous pixel storage
    uint32_t const* ptr = reinterpret_cast<uint32_t const*>(pixels);
    _pixels = RGBA32::Vector(ptr, ptr + (_raw_w * _raw_h));

    _updatePlanesScaling();
}
__CATCH_AND_RETHROW_METHOD_EXC

void Texture::useFile(std::string filepath)
{
    _type = Type::Raw;
    _loading_thread.run(filepath);
}

void Texture::setType(Type type) noexcept
{
    if (type == _type) {
        return;
    }
    _type = type;
    if (type == Type::Raw) {
        if (_pixels.empty()) {
            return;
        }
        _updatePlanesScaling();
        _raw_texture.edit(&_pixels.at(0), _raw_w, _raw_h);
    }
    else if (type == Type::Text) {
        TR::Area::Ptr const& text_area = getTextArea();
        if (!text_area) {
            _text_w = 0;
            _text_h = 0;
            _updatePlanesScaling();
            _raw_texture.edit(nullptr, 0, 0);
        }
        else {
            text_area->getDimensions(_text_w, _text_h);
            _updatePlanesScaling();
            _raw_texture.edit(text_area->pixelsGet(), _text_w, _text_h);
        }
    }
}

void Texture::setTextAreaID(uint32_t id)
{
    if (_text_area_id == id) {
        return;
    }
    _text_area_id = id;
    TR::Area::Ptr const& text_area = getTextArea();
    if (_type == Type::Text && text_area) {
        int w, h;
        text_area->getDimensions(w, h);
        _internal_edit(text_area->pixelsGet(), w, h);
    }
}

TR::Area::Ptr const& Texture::getTextArea() const noexcept
{
    TR::Area::Map const& text_areas = TR::Area::getMap();
    if (text_areas.count(_text_area_id) == 0) {
        static const TR::Area::Ptr empty_ptr(nullptr);
        return empty_ptr;
    }
    return text_areas.at(_text_area_id);
}

void Texture::getDimensions(int& w, int& h) const noexcept
{
    if (_type == Type::Raw) {
        w = _raw_w;
        h = _raw_h;
    }
    else if (_type == Type::Text) {
        w = _text_w;
        h = _text_h;
    }
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
    for (auto it = objects.planes.cbegin(); it != objects.planes.cend(); ++it) {
        Plane::Ptr const& plane = it->second;
        if (plane->_use_texture && plane->_texture_id == id) {
            plane->_updateTexScaling();
        }
    }
}

void Texture::_internal_edit(void const* pixels, int w, int h)
{
    if (_type == Type::Raw) {
        if (w != _raw_w || h != _raw_h) {
            _raw_w = w;
            _raw_h = h;
            _updatePlanesScaling();
        }
    }
    else if (_type == Type::Text) {
        if (w != _text_w || h != _text_h) {
            _text_w = w;
            _text_h = h;
            _updatePlanesScaling();
        }
    }
    _raw_texture.edit(pixels, w, h);
}

void Texture::_AsyncLoading::_asyncFunction(std::string filepath)
{
    // Load image
    SSS::C_Ptr <uint32_t, void(*)(void*), stbi_image_free>
        raw_pixels(reinterpret_cast<uint32_t*>(stbi_load(
            filepath.c_str(),   // Filepath to picture
            &_w,                // Width, to query
            &_h,                // Height, to query
            nullptr,            // Byte composition, to query if not requested
            4                   // Byte composition, to request (here RGBA32)
        )));
    // Throw if error
    if (raw_pixels == nullptr) {
        SSS::throw_exc(stbi_failure_reason());
    }
    // Fill vector
    if (_beingCanceled()) return;
    _pixels = RGBA32::Vector(raw_pixels.get(), raw_pixels.get() + (_w * _h));
}

__SSS_GL_END