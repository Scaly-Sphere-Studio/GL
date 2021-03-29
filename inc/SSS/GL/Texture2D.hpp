#pragma once

#include "_pointers.hpp"

__SSS_GL_BEGIN

class Texture2D {
    friend class Window;
    friend class Plane;

public:
    using Shared = std::shared_ptr<Texture2D>;
    using Ptr = std::unique_ptr<Texture2D>;

    Texture2D();
    Texture2D(std::string filepath);
    Texture2D(unsigned char const* pixels, int width, int height);

    void useFile(std::string filepath);
    void edit(unsigned char const* pixels, int width, int height);

    inline void getDimensions(int& width, int& height) const {
        width = _w;
        height = _h;
    };
    inline std::vector<bool> const& getAlphaMap() const {
        return _alpha_map;
    };
    inline void bind() const {
        _raw_texture.bind();
    };

private:
    int _w{ 0 }, _h{ 0 };
    _internal::Texture _raw_texture{ GL_TEXTURE_2D };
    std::vector<bool> _alpha_map;
};

__SSS_GL_END