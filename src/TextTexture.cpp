#include "SSS/GL/TextTexture.hpp"
#include "SSS/GL/Window.hpp"

__SSS_GL_BEGIN

std::vector<TextTexture::Weak> TextTexture::_instances{};

TextTexture::TextTexture(std::shared_ptr<Window> window, int width, int height)
    : TextureBase(window, GL_TEXTURE_2D), TR::TextArea(width, height)
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
    cleanWeakPtrVector(_instances);
}

TextTexture::Shared TextTexture::create(std::shared_ptr<Window> window, int width, int height)
{
    // Use new instead of std::make_shared to access private constructor
    Shared texture = (Shared)_instances.emplace_back(Shared(new TextTexture(window, width, height)));
    TR::TextArea::_instances.push_back(std::static_pointer_cast<TR::TextArea>(texture));
    return texture;
}

void TextTexture::bind()
{
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