#include "GL/Objects/Texture.hpp"
#include "GL/Objects/Models/Plane.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#pragma warning(suppress : 4996)
#include <stb_image_write.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include <stb_image.h>

SSS_GL_BEGIN;

std::string Texture::_resource_folder;


void Texture::_register()
{
    REGISTER_EVENT("SSS_TEXTURE_RESIZE"); 
    REGISTER_EVENT("SSS_TEXTURE_CONTENT"); 
    REGISTER_EVENT("SSS_TEXTURE_LOADED"); 
}

Texture::Texture() try
    : _raw_texture(GL_TEXTURE_2D_ARRAY)
{
    _raw_texture.bind();
    _raw_texture.parameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    _raw_texture.parameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    _raw_texture.parameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    _raw_texture.parameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    _observe(_loading_thread);

    // Log
    if (Log::GL::Texture::query(Log::GL::Texture::get().life_state)) {
        LOG_GL_MSG("Texture -> created");
    }
}
CATCH_AND_RETHROW_METHOD_EXC;

Texture::~Texture()
{
    // Log
    if (Log::GL::Texture::query(Log::GL::Texture::get().life_state)) {
        LOG_GL_MSG("Texture -> deleted");
    }
}

Texture::Shared Texture::create(std::string const& filepath)
{
    Shared ret = create();
    ret->loadImage(filepath);
    return ret;
}

Texture::Shared Texture::create(TR::Area::Shared area)
{
    Shared ret = create();
    ret->setTextArea(area);
    return ret;
}

void Texture::setType(Type type) noexcept
{
    if (_type != type)
        _internalEdit(type);
}

void Texture::loadImage(std::string const& filepath)
{
    _loading_thread.run(_resource_folder, filepath);
    _filepath = filepath;
}

void Texture::editRawPixels(void const* pixels, int width, int height) try
{
    if (_frames.size() != 1) {
        _frames.resize(1);
    }
    _frames.w = width;
    _frames.h = height;
    // Replace previous pixel storage
    uint32_t const* ptr = reinterpret_cast<uint32_t const*>(pixels);
    _frames[0].pixels = RGBA32::Vector(ptr, ptr + (width * height));

    // Update plane type and scaling
    _internalEdit(Type::Raw);

    // Log
    if (Log::GL::Texture::query(Log::GL::Texture::get().edit)) {
        LOG_GL_MSG("Texture -> edit");
    }
}
CATCH_AND_RETHROW_METHOD_EXC;

void Texture::setColor(RGBA32 color)
{
    editRawPixels(&color, 1, 1);
}

void Texture::setTextArea(TR::Area::Shared area)
{
    _set(_area, area);
    _internalEdit(Type::Text);
}

void Texture::getCurrentDimensions(int& w, int& h) const noexcept
{
    if (_type == Type::Raw) {
        w = _frames.w;
        h = _frames.h;
    }
    else if (_type == Type::Text) {
        if (_area)
            _area->pixelsGetDimensions(w, h);
        else {
            w = 0;
            h = 0;
        }
    }
}

std::tuple<int, int> Texture::getCurrentDimensions() const noexcept
{
    int w, h;
    getCurrentDimensions(w, h);
    return std::make_tuple(w, h);
}

void Texture::savePNG() const
{
    int w, h;
    getCurrentDimensions(w, h);
    std::vector<uint8_t> pixels(4 * w * h);

    glGetTextureImage(_raw_texture.id, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.size(), &pixels[0]);

    std::string time = std::format("{:%Y-%m-%d_%H-%M-%S}", std::chrono::system_clock::now());
    size_t const index = time.find('.');
    if (index < time.size()) {
        time.resize(index);
    }

    std::string const name = time + "_" + THIS_ADDR + ".png";
    stbi_write_png(name.c_str(), w, h, 4, &pixels[0], 0);
}

void Texture::_subjectUpdate(Subject const& subject, int event_id)
{
    if (subject.is<_AsyncLoading>()) {
        _frames = std::move(_loading_thread._frames);
        _internalEdit(Type::Raw);
        EMIT_EVENT("SSS_TEXTURE_LOADED");
    }
    else if (subject.is<TR::Area>()) {
        _internalEdit(Type::Text);
    }
}

void Texture::_AsyncLoading::_asyncFunction(std::string folder, std::string filepath)
{
    _frames.clear();
    _frames.total_time = std::chrono::nanoseconds(0);
    _frames.w = 0;
    _frames.h = 0;

    std::string path;
    if (path = folder + filepath; folder.empty() || !pathIsFile(path)) {
        if (path = pathWhich(filepath); !pathIsFile(path)) {
            throw_exc(CONTEXT_MSG("Found no file for given arguments", filepath));
        }
    }

    // Check if filepath ends with ".png"
    // If file is a PNG or APNG, use load_apng (which works for simple PNG files)
    // Else, use stbi functions
    static const std::string png(".png");
    // Ends with ".png"
    if (filepath.compare(filepath.length() - png.length(), png.length(), png) == 0) {
        // Load frames
        std::vector<_internal::APNGFrame> apng_frames;
        if (load_apng(path.c_str(), apng_frames) < 0) {
            SSS::throw_exc(CONTEXT_MSG("load_apng error", filepath));
        }
        // Copy data
        if (!apng_frames.empty()) {
            _frames.w = apng_frames[0].w;
            _frames.h = apng_frames[0].h;
            _frames.reserve(apng_frames.size());
            // Copy each frame one by one
            for (auto& apng_frame : apng_frames) {
                if (_beingCanceled()) return;
                // Copy pixels and compute delay
                auto& frame = _frames.emplace_back();
                uint32_t const* p = reinterpret_cast<uint32_t const*>(apng_frame.vec.data());
                frame.pixels = RGBA32::Vector(p, p + (_frames.w * _frames.h));
                if (apng_frame.delay_den > 0) {
                    frame.delay = std::chrono::nanoseconds(static_cast<int64_t>(1e9
                        * static_cast<double>(apng_frame.delay_num)
                        / static_cast<double>(apng_frame.delay_den)
                        ));
                }
                else
                    frame.delay = std::chrono::milliseconds(16);
                _frames.total_time += frame.delay;
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
                path.c_str(),   // Filepath to picture
                &_frames.w,     // Width, to query
                &_frames.h,     // Height, to query
                nullptr,        // Byte composition, to query if not requested
                4               // Byte composition, to request (here RGBA32)
            )));
        // Throw if error
        if (raw_pixels == nullptr) {
            SSS::throw_exc(CONTEXT_MSG(stbi_failure_reason(), filepath));
        }
        // Fill vector
        if (_beingCanceled()) return;
        _frames.emplace_back().pixels =
            RGBA32::Vector(raw_pixels.get(), raw_pixels.get() + (_frames.w * _frames.h));
    }
}

void Texture::_internalEdit(Type type)
{
    _type = type;
    if (_type == Type::Raw) {
        if (_raw_texture.editSettings(_frames.w, _frames.h, static_cast<int>(_frames.size())))
        {
            EMIT_EVENT("SSS_TEXTURE_RESIZE");
        }
        for (uint32_t i = 0; i < _frames.size(); ++i) {
            _raw_texture.editPixels(_frames[i].pixels.data(), i);
        }
    }
    else if (_type == Type::Text) {
        int w = 0, h = 0;
        if (_area)
            _area->pixelsGetDimensions(w, h);
        if (_raw_texture.editSettings(w, h))
        {
            EMIT_EVENT("SSS_TEXTURE_RESIZE");
        }
        if (_area)
            _raw_texture.editPixels(_area->pixelsGet());
    }

    if (_callback_f)
        _callback_f(*this);

    EMIT_EVENT("SSS_TEXTURE_CONTENT"); 
    // Log
    if (Log::GL::Texture::query(Log::GL::Texture::get().edit)) {
        LOG_GL_MSG("Texture -> edit");
    }
}


SSS_GL_END;