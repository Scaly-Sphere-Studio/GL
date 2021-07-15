#include "SSS/GL/Texture2D.hpp"
#include "SSS/GL/Window.hpp"

// Init STB
#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include <stb_image.h>

__SSS_GL_BEGIN

// Init statics
std::vector<Texture2D::Weak> Texture2D::_instances{};
bool Texture2D::LOG::constructor{ false };
bool Texture2D::LOG::destructor{ false };

Texture2D::Texture2D(std::shared_ptr<Window> window) try
    : TextureBase(window, GL_TEXTURE_2D)
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
    cleanWeakPtrVector(_instances);
    if (LOG::destructor) {
        __LOG_DESTRUCTOR
    }
}

Texture2D::Shared Texture2D::create(std::shared_ptr<Window> window) try
{
    // Use new instead of std::make_shared to access private constructor
    return (Shared)_instances.emplace_back(Texture2D::Shared(new Texture2D(window)));
}
__CATCH_AND_RETHROW_FUNC_EXC

Texture2D::Shared Texture2D::create(std::shared_ptr<Window> window,
    std::string const& filepath) try
{
    // Use new instead of std::make_shared to access private constructor
    Shared texture = (Shared)_instances.emplace_back(Texture2D::Shared(new Texture2D(window)));
    texture->useFile(filepath);
    return texture;
}
__CATCH_AND_RETHROW_FUNC_EXC

Texture2D::Shared Texture2D::create(std::shared_ptr<Window> window,
    void const* pixels, int width, int height) try
{
    // Use new instead of std::make_shared to access private constructor
    Shared texture = (Shared)_instances.emplace_back(Texture2D::Shared(new Texture2D(window)));
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
    _tex_w = width;
    _tex_h = height;
    // Give the image to the OpenGL texture
    _raw_texture.edit(pixels, _tex_w, _tex_h);
    // Clear previous pixel storage
    _pixels.clear();

    _updatePlanesScaling();
}
__CATCH_AND_RETHROW_METHOD_EXC

void Texture2D::_updatePlanesScaling()
{
    // Update texture scaling of all planes & buttons
    TextureBase::Shared texture = std::static_pointer_cast<TextureBase>(shared_from_this());
    for (Plane::Weak const& weak_plane : Plane::_instances) {
        Plane::Shared plane = weak_plane.lock();
        if (plane && plane->_texture == texture)
            plane->_updateTexScaling();
    }
    for (Button::Weak const& weak_button : Button::_instances) {
        Button::Shared button = weak_button.lock();
        if (button && button->_texture == texture)
            button->_updateTexScaling();
    }
}

void Texture2D::_LoadingThread::_function(Texture2D::Weak texture, std::string filepath)
{
    auto& _pixels = texture.lock()->_pixels;
    auto& _tex_w = texture.lock()->_tex_w;
    auto& _tex_h = texture.lock()->_tex_h;

    // Load image
    SSS::C_Ptr <uint32_t, void(*)(void*), stbi_image_free>
        raw_pixels((uint32_t*)(stbi_load(
            filepath.c_str(),   // Filepath to picture
            &_tex_w,                // Width, to query
            &_tex_h,                // Height, to query
            nullptr,            // Byte composition, to query if not requested
            4                   // Byte composition, to request
        )));
    // Throw if error
    if (raw_pixels == nullptr) {
        SSS::throw_exc(stbi_failure_reason());
    }
    // Fill vector
    if (_is_canceled) return;
    _pixels = RGBA32::Pixels(raw_pixels.get(), raw_pixels.get() + (_tex_w * _tex_h));
}

void Texture2D::pollThreads()
{
    // Loop over each Texture2D instance
    for (Weak const& weak : _instances) {
        Shared texture = weak.lock();
        if (!texture) {
            continue;
        }
        // If the loading thread is pending, edit the texture
        if (texture->_loading_thread.isPending()) {
            // Give the image to the OpenGL texture and notify all planes & buttons
            texture->_raw_texture.edit(&texture->_pixels[0], texture->_tex_w, texture->_tex_h);
            texture->_updatePlanesScaling();
            // Set thread as handled.
            texture->_loading_thread.setAsHandled();
        }
    }
}

__SSS_GL_END