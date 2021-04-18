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

std::deque<Texture2D::Weak> Texture2D::_instances{};

Texture2D::Texture2D() try
{
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

Texture2D::Shared Texture2D::create() try
{
    Shared texture(new Texture2D());
    _instances.push_back(texture->weak_from_this());
    return texture;
}
__CATCH_AND_RETHROW_FUNC_EXC

Texture2D::Shared Texture2D::create(std::string const& filepath) try
{
    Shared texture = create();
    texture->useFile(filepath);
    return texture;
}
__CATCH_AND_RETHROW_FUNC_EXC

Texture2D::Shared Texture2D::create(void const* pixels, int width, int height) try
{
    Shared texture = create();
    texture->edit(pixels, width, height);
    return texture;
}
__CATCH_AND_RETHROW_FUNC_EXC

void Texture2D::useFile(std::string filepath) try
{
    _loading_thread.run(weak_from_this(), filepath);
}
__CATCH_AND_RETHROW_METHOD_EXC

void Texture2D::edit(void const* pixels, int width, int height) try
{
    // Update size info
    _w = width;
    _h = height;
    // Give the image to the OpenGL texture
    _raw_texture.edit(pixels, _w, _h);
    // Clear previous pixel storage
    _pixels.clear();
    // Update texture scaling of all planes (they are stored in window instances)
    Window::_textureWasEdited(shared_from_this());
}
__CATCH_AND_RETHROW_METHOD_EXC

void Texture2D::_LoadingThread::_function(Texture2D::Weak texture, std::string filepath)
{
    auto& _pixels = texture.lock()->_pixels;
    auto& _w = texture.lock()->_w;
    auto& _h = texture.lock()->_h;

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

void Texture2D::pollThreads()
{
    for (Weak const& weak : _instances) {
        Shared texture = weak.lock();
        if (texture) {
            if (texture->_loading_thread.isPending()) {
                // Give the image to the OpenGL texture
                texture->_raw_texture.edit(&texture->_pixels[0], texture->_w, texture->_h);
                // Update texture scaling of all planes (they are stored in window instances)
                Window::_textureWasEdited(texture);
                // Set thread as handled.
                texture->_loading_thread.setAsHandled();
            }
        }
    }
}

__SSS_GL_END