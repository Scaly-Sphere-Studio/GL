#include "SSS/GL/Objects/Texture.hpp"
#include "SSS/GL/Objects/Models/Plane.hpp"

// Init STB
#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include <stb_image.h>

SSS_GL_BEGIN;

Texture::Texture(std::shared_ptr<Window> window, uint32_t id) try
    : _internal::WindowObjectWithID(window, id), _raw_texture(window, GL_TEXTURE_2D_ARRAY)
{
    Context const context = _get_context();
    _raw_texture.bind();
    _raw_texture.parameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    _raw_texture.parameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    _raw_texture.parameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    _raw_texture.parameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    
    // Log
    if (Log::GL::Texture::query(Log::GL::Texture::get().life_state)) {
        char buff[256];
        sprintf_s(buff, "'%s' -> Texture (id: %04u) -> created",
            WINDOW_TITLE(_get_window()), _id);
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
            WINDOW_TITLE(_get_window()), _id);
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

Texture& Texture::create(std::string const& filepath)
{
    Texture& ret = create();
    ret.loadImage(filepath);
    return ret;
}

Texture& Texture::create(TR::Area const& area)
{
    Texture& ret = create();
    ret.setTextArea(area);
    return ret;
}

void Texture::setType(Type type) noexcept
{
    if (type == _type) {
        return;
    }
    _type = type;
    if (type == Type::Raw) {
        auto pixels = getRawPixels();
        if (pixels.empty()) {
            return;
        }
        _updatePlanes();
        _raw_texture.editSettings(_raw_w, _raw_h, static_cast<uint32_t>(_frames.size()));
        _raw_texture.editPixels(pixels.data());
    }
    else if (type == Type::Text) {
        TR::Area* text_area = getTextArea();
        if (text_area) {
            text_area->pixelsGetDimensions(_text_w, _text_h);
            _updatePlanes();
            _raw_texture.editSettings(_text_w, _text_h);
            _raw_texture.editPixels(text_area->pixelsGet());
        }
        else {
            _text_w = 0;
            _text_h = 0;
            _updatePlanes();
        }
    }
}

void Texture::loadImage(std::string const& filepath)
{
    _loading_thread.run(filepath);
    _filepath = filepath;
}

void Texture::editRawPixels(void const* pixels, int width, int height) try
{
    // Update size info
    _raw_w = width;
    _raw_h = height;
    // Give the image to the OpenGL texture
    _raw_texture.editSettings(width, height);
    _raw_texture.editPixels(pixels);
    if (_frames.size() != 1) {
        _frames.resize(1);
    }
    // Replace previous pixel storage
    uint32_t const* ptr = reinterpret_cast<uint32_t const*>(pixels);
    _frames[0].pixels = RGBA32::Vector(ptr, ptr + (_raw_w * _raw_h));

    // Update plane type and scaling
    _type = Type::Raw;
    _updatePlanes();

    // Log
    if (Log::GL::Texture::query(Log::GL::Texture::get().edit)) {
        char buff[256];
        sprintf_s(buff, "'%s' -> Texture (id: %04u) -> edit",
            WINDOW_TITLE(_get_window()), _id);
        LOG_GL_MSG(buff);
    }
}
CATCH_AND_RETHROW_METHOD_EXC;

void Texture::setTextAreaID(uint32_t id)
{
    _text_area_id = id;
    TR::Area* text_area = getTextArea();
    if (text_area) {
        _type = Type::Text;
        int w, h;
        text_area->pixelsGetDimensions(w, h);
        _internalEdit(text_area->pixelsGet(), w, h);
    }
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

std::tuple<int, int> Texture::getCurrentDimensions() const noexcept
{
    int w, h;
    getCurrentDimensions(w, h);
    return std::make_tuple(w, h);
}

void Texture::_AsyncLoading::_asyncFunction(std::string filepath)
{
    _frames.clear();
    _total_frames_time = std::chrono::nanoseconds(0);
    _w = 0;
    _h = 0;
    // Check if filepath ends with ".png"
    // If file is a PNG or APNG, use load_apng (which works for simple PNG files)
    // Else, use stbi functions
    static const std::string png(".png");
    // Ends with ".png"
    if (filepath.compare(filepath.length() - png.length(), png.length(), png) == 0) {
        // Load frames
        std::vector<_internal::APNGFrame> apng_frames;
        if (load_apng(filepath.c_str(), apng_frames) < 0) {
            SSS::throw_exc(CONTEXT_MSG("load_apng error", filepath));
        }
        // Copy data
        if (!apng_frames.empty()) {
            _w = apng_frames[0].w;
            _h = apng_frames[0].h;
            _frames.reserve(apng_frames.size());
            // Copy each frame one by one
            for (auto& apng_frame : apng_frames) {
                if (_beingCanceled()) return;
                // Copy pixels and compute delay
                auto& frame = _frames.emplace_back();
                uint32_t const* p = reinterpret_cast<uint32_t const*>(apng_frame.vec.data());
                frame.pixels = RGBA32::Vector(p, p + (_w * _h));
                if (apng_frame.delay_den > 0) {
                    frame.delay = std::chrono::nanoseconds(static_cast<int64_t>(1e9
                        * static_cast<double>(apng_frame.delay_num)
                        / static_cast<double>(apng_frame.delay_den)
                        ));
                }
                else
                    frame.delay = std::chrono::milliseconds(16);
                _total_frames_time += frame.delay;
                // Free raw pixels early
                apng_frame.vec.clear();
                apng_frame.rows.clear();
            }
        }
    }
    // Doesn't end with ".png"
    else {
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
        _frames.emplace_back().pixels =
            RGBA32::Vector(raw_pixels.get(), raw_pixels.get() + (_w * _h));
    }
}

void Texture::_updatePlanes()
{
    Window::Shared const window = _get_window();
    // Update texture scaling of all planes & buttons matching this texture
    for (Plane::Weak const& weak : Plane::_instances) {
        Plane::Shared plane = weak.lock();
        if (plane->_get_window() == window
            && plane->_use_texture
            && plane->_texture_id == _id)
        {
            plane->_updateTexScaling();
            plane->_animation_duration = std::chrono::nanoseconds(0);
            plane->_texture_offset = 0;
        }
    }
}

void Texture::_internalEdit(void const* pixels, int w, int h)
{
    if (_type == Type::Raw) {
        if (w != _raw_w || h != _raw_h) {
            _raw_w = w;
            _raw_h = h;
            _updatePlanes();
        }
    }
    else if (_type == Type::Text) {
        if (w != _text_w || h != _text_h) {
            _text_w = w;
            _text_h = h;
            _updatePlanes();
        }
    }
    _raw_texture.editSettings(w, h);
    _raw_texture.editPixels(pixels);
    // Log
    if (Log::GL::Texture::query(Log::GL::Texture::get().edit)) {
        char buff[256];
        sprintf_s(buff, "'%s' -> Texture (id: %04u) -> edit",
            WINDOW_TITLE(_get_window()), _id);
        LOG_GL_MSG(buff);
    }
}

SSS_GL_END;