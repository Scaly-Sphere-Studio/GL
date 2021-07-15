#pragma once

#include "Texture2D.hpp"

__SSS_GL_BEGIN

class Window;

class TextTexture :
    public TextureBase,
    public TR::TextArea,
    public std::enable_shared_from_this<TextTexture>
{
    friend class Window;
private:
    TextTexture(std::shared_ptr<Window> window, int width, int height);
    using Weak = std::weak_ptr<TextTexture>;

private:
    static std::vector<Weak> _instances;

public:
    ~TextTexture();
    
    using Shared = std::shared_ptr<TextTexture>;
    static Shared create(std::shared_ptr<Window> window, int width, int height);

    virtual void bind();

    bool scroll(int pixels) noexcept;

private:
    bool _update_texture{ false };
};

__SSS_GL_END