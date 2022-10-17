#include "SSS/GL/Objects/Texture.hpp"
#include "SSS/GL/Objects/Models/Plane.hpp"

// Init STB
#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include <stb_image.h>

SSS_GL_BEGIN;

Texture::Texture(std::weak_ptr<Window> window, uint32_t id) try
    : _internal::WindowObjectWithID(window, id), _raw_texture(window, GL_TEXTURE_2D)
{
    Context const context(_window);
    _raw_texture.bind();
    _raw_texture.parameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    _raw_texture.parameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    _raw_texture.parameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    _raw_texture.parameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    
    // Log
    if (Log::GL::Texture::query(Log::GL::Texture::get().life_state)) {
        char buff[256];
        sprintf_s(buff, "'%s' -> Texture (id: %04u) -> created",
            WINDOW_TITLE(_window), _id);
        LOG_GL_MSG(buff);
    }
}
CATCH_AND_RETHROW_METHOD_EXC;

Texture::~Texture()
{
    // Log
    if (Log::GL::Texture::query(Log::GL::Texture::get().life_state)) {
        char buff[256];
        sprintf_s(buff, "'%s' -> Texture (id: %04u) -> deleted",
            WINDOW_TITLE(_window), _id);
        LOG_GL_MSG(buff);
    }
}

Texture& Texture::create(std::shared_ptr<Window> win) try
{
    if (!win) {
        throw_exc("Given Window is nullptr.");
    }
	return win->createTexture();
}
CATCH_AND_RETHROW_FUNC_EXC;

Texture& Texture::create() try
{
    std::shared_ptr<Window> win = Window::getFirst();
    if (!win) {
        throw_exc("No Window instance exists.");
    }
	return win->createTexture();
}
CATCH_AND_RETHROW_FUNC_EXC;

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
        TR::Area* text_area = getTextArea();
        if (text_area) {
            text_area->pixelsGetDimensions(_text_w, _text_h);
            _updatePlanesScaling();
            _raw_texture.edit(text_area->pixelsGet(), _text_w, _text_h);
        }
        else {
            _text_w = 0;
            _text_h = 0;
            _updatePlanesScaling();
            _raw_texture.edit(nullptr, 0, 0);
        }
    }
}

void Texture::loadImage(std::string const& filepath)
{
    _loading_thread.run(filepath);
}

void Texture::editRawPixels(void const* pixels, int width, int height) try
{
    // Update size info
    _raw_w = width;
    _raw_h = height;
    // Give the image to the OpenGL texture
    _raw_texture.edit(pixels, _raw_w, _raw_h);
    // Replace previous pixel storage
    uint32_t const* ptr = reinterpret_cast<uint32_t const*>(pixels);
    _pixels = RGBA32::Vector(ptr, ptr + (_raw_w * _raw_h));

    // Update plane scaling only if immediately needed
    if (_type == Type::Raw) {
        _updatePlanesScaling();
    }

    // Log
    if (Log::GL::Texture::query(Log::GL::Texture::get().edit)) {
        char buff[256];
        sprintf_s(buff, "'%s' -> Texture (id: %04u) -> edit",
            WINDOW_TITLE(_window), _id);
        LOG_GL_MSG(buff);
    }
}
CATCH_AND_RETHROW_METHOD_EXC;

void Texture::setTextAreaID(uint32_t id)
{
    if (_text_area_id == id) {
        return;
    }
    _text_area_id = id;
    TR::Area* text_area = getTextArea();
    if (_type == Type::Text && text_area) {
        int w, h;
        text_area->pixelsGetDimensions(w, h);
        _internalEdit(text_area->pixelsGet(), w, h);
    }
}

TR::Area* Texture::getTextArea() const noexcept
{
    return TR::Area::get(_text_area_id);
}

void Texture::getCurrentDimensions(int& w, int& h) const noexcept
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

void Texture::_AsyncLoading::_asyncFunction(std::string filepath)
{
    // Load image
    SSS::C_Ptr <uint32_t, void(*)(void*), stbi_image_free>
        raw_pixels(reinterpret_cast<uint32_t*>(stbi_load(
            pathWhich(filepath).c_str(),    // Filepath to picture
            &_w,        // Width, to query
            &_h,        // Height, to query
            nullptr,    // Byte composition, to query if not requested
            4           // Byte composition, to request (here RGBA32)
        )));
    // Throw if error
    if (raw_pixels == nullptr) {
        SSS::throw_exc(CONTEXT_MSG(stbi_failure_reason(), filepath));
    }
    // Fill vector
    if (_beingCanceled()) return;
    _pixels = RGBA32::Vector(raw_pixels.get(), raw_pixels.get() + (_w * _h));
}

void Texture::_updatePlanesScaling()
{
    Window::Shared const window = _window.lock();
    if (!window) {
        return;
    }
    // Update texture scaling of all planes & buttons matching this texture
    for (Plane::Weak const& weak : Plane::_instances) {
        Plane::Shared plane = weak.lock();
        if (plane->_window.lock() == window
            && plane->_use_texture
            && plane->_texture_id == _id)
        {
            plane->_updateTexScaling();
        }
    }
}

void Texture::_internalEdit(void const* pixels, int w, int h)
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
    // Log
    if (Log::GL::Texture::query(Log::GL::Texture::get().edit)) {
        char buff[256];
        sprintf_s(buff, "'%s' -> Texture (id: %04u) -> edit",
            WINDOW_TITLE(_window), _id);
        LOG_GL_MSG(buff);
    }
}

SSS_GL_END;