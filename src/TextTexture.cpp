#include "SSS/GL/TextTexture.hpp"
#include "SSS/GL/Window.hpp"

__SSS_GL_BEGIN

TextTexture::TextTexture(std::weak_ptr<Window> window, int width, int height)
    : TextureBase(window, GL_TEXTURE_2D), TR::TextArea(width, height)
{
    Context const context(_window);
    _tex_w = width;
    _tex_h = height;
    _raw_texture.bind();
    _raw_texture.parameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    _raw_texture.parameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    _raw_texture.parameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    _raw_texture.parameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
}

TextTexture::~TextTexture()
{
}

void TextTexture::bind()
{
    Context const context(_window);
    if (update() || _update_texture) {
        _raw_texture.edit(getPixels(), _tex_w, _tex_h);
        _update_texture = false;
    }
    _raw_texture.bind();
}

bool TextTexture::scroll(int pixels) noexcept
{
    bool ret = TR::TextArea::scroll(pixels);
    _update_texture = _update_texture || ret;
    return ret;
}

__SSS_GL_END