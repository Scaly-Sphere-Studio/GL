#include "SSS/GL/TextTexture.hpp"
#include "SSS/GL/Context.hpp"

__SSS_GL_BEGIN

TextTexture::TextTexture(std::shared_ptr<Context> context, int width, int height)
    : TextureBase(context, GL_TEXTURE_2D), TR::TextArea(width, height)
{
    ContextManager const context_manager(_context.lock());
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
    ContextManager const context_manager(_context.lock());
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