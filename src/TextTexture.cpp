#include "SSS/GL/TextTexture.hpp"

__SSS_GL_BEGIN

TextTexture::TextTexture(int width, int height)
    : TextureBase(GL_TEXTURE_2D), TR::TextArea(width, height)
{
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

TextTexture::Shared TextTexture::create(int width, int height)
{
    Shared shared(new TextTexture(width, height));
    TR::TextArea::_instances.push_back(std::static_pointer_cast<TR::TextArea>(shared));
    return shared;
}

void TextTexture::bind()
{
    if (willDraw() || _update_texture) {
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