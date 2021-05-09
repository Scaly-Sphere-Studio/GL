#include "SSS/GL/TextTexture.hpp"

__SSS_GL_BEGIN

TextTexture::TextTexture(int width, int height)
    : TextureBase(GL_TEXTURE_2D)
{
    _w = width;
    _h = height;
    _text_area = TR::TextArea::create(width, height);
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
    return Shared(new TextTexture(width, height));
}

void TextTexture::bind()
{
    if (_text_area->willDraw() || _update_texture) {
        _raw_texture.edit(_text_area->getPixels(), _w, _h);
        _update_texture = false;
    }
    _raw_texture.bind();
}

void TextTexture::clear() noexcept
{
    _text_area->clear();
}

void TextTexture::useBuffer(TR::Buffer::Shared buffer)
{
    _text_area->useBuffer(buffer);
}

void TextTexture::scroll(int pixels) noexcept
{
    _update_texture = _text_area->scroll(pixels);
}

void TextTexture::setTypeWriter(bool activate) noexcept
{
    _text_area->setTypeWriter(activate);
}

bool TextTexture::incrementCursor() noexcept
{
    return _text_area->incrementCursor();
}

__SSS_GL_END