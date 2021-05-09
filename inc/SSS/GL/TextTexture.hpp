#pragma once

#include "Texture2D.hpp"

__SSS_GL_BEGIN

class TextTexture : public TextureBase, public std::enable_shared_from_this<TextTexture> {
private:
    TextTexture(int width, int height);

public:
    ~TextTexture();
    
    using Shared = std::shared_ptr<TextTexture>;
    static Shared create(int width, int height);

    virtual void bind();

    void clear() noexcept;
    void useBuffer(TR::Buffer::Shared buffer);
    void scroll(int pixels) noexcept;
    void setTypeWriter(bool activate) noexcept;
    bool incrementCursor() noexcept;

private:
    bool _update_texture{ true };
    TR::TextArea::Shared _text_area;
};

__SSS_GL_END