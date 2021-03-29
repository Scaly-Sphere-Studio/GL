#include "SSS/GL/Texture2D.hpp"
#include "SSS/GL/Window.hpp"

// Init STB
#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include <stb_image.h>

__SSS_GL_BEGIN

Texture2D::Texture2D() try
{
    _raw_texture.bind();
    _raw_texture.parameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    _raw_texture.parameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    _raw_texture.parameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    _raw_texture.parameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
}
__CATCH_AND_RETHROW_METHOD_EXC

Texture2D::Texture2D(std::string filepath) try
    : Texture2D()
{
    useFile(filepath);
}
__CATCH_AND_RETHROW_METHOD_EXC

Texture2D::Texture2D(unsigned char const* pixels, int width, int height) try
    : Texture2D()
{
    edit(pixels, width, height);
}
__CATCH_AND_RETHROW_METHOD_EXC

void Texture2D::useFile(std::string filepath) try
{
    // Load image
    SSS::C_Ptr<unsigned char, void(*)(void*), stbi_image_free> data
        = stbi_load(filepath.c_str(), &_w, &_h, nullptr, 4);
    // Throw if error
    if (!data) {
        SSS::throw_exc(stbi_failure_reason());
    }
    edit(data.get(), _w, _h);
}
__CATCH_AND_RETHROW_METHOD_EXC

void Texture2D::edit(unsigned char const* pixels, int width, int height) try
{
    // Update size info
    _w = width;
    _h = height;

    // Give the image to the OpenGL texture
    _raw_texture.edit(pixels, _w, _h);

    // Generate alpha map
    _alpha_map.resize(_w * _h);
    for (size_t i = 0; i < _alpha_map.size(); ++i) {
        _alpha_map[i] = pixels[i * 4 + 3] != 0;
    }
}
__CATCH_AND_RETHROW_METHOD_EXC

__SSS_GL_END