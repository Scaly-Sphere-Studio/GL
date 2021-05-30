#pragma once

#include "Texture2D.hpp"

__SSS_GL_BEGIN

class TextTexture :
    public TextureBase,
    public TR::TextArea,
    public std::enable_shared_from_this<TextTexture> {
private:
    TextTexture(int width, int height);

public:
    ~TextTexture();
    
    using Shared = std::shared_ptr<TextTexture>;
    static Shared create(int width, int height);

    virtual void bind();

    bool scroll(int pixels) noexcept;

private:
    bool _update_texture{ false };
};

__SSS_GL_END