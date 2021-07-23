#pragma once

#include "_internal/pointers.hpp"

__SSS_GL_BEGIN

class Window;

enum class TextureType {
    None = 0,   // No class
    Classic,    // Texture2D class
    Text        // TextTexture class
};

class TextureBase : public _internal::ContextObject {
protected:
    TextureBase(GLFWwindow const* context, GLenum target)
        : _internal::ContextObject(context), _raw_texture(context, target) {};
public:
    TextureBase() = delete;
    virtual void bind() { _raw_texture.bind(); };
    inline void getDimensions(int& w, int& h) const noexcept { w = _tex_w; h = _tex_h; };
    inline RGBA32::Pixels const& getStoredPixels() { return _raw_pixels; };
protected:
    int _tex_w{ 0 }, _tex_h{ 0 };
    _internal::Texture _raw_texture;
    RGBA32::Pixels _raw_pixels;
};

class Texture2D : public TextureBase {
    friend class Plane;
    friend class Context;

protected:
    Texture2D(GLFWwindow const* context);

public:
    ~Texture2D();

    struct LOG {
        static bool constructor;
        static bool destructor;
    };

    using Ptr = std::unique_ptr<Texture2D>;
    
    void useFile(std::string filepath);
    void edit(void const* pixels, int width, int height);

private:
// --- Image loading ---

    // Loading thread which fills _raw_pixels using stb_image
    class _LoadingThread : public ThreadBase <std::string> {
        friend class Context;
        // Export constructors
        using ThreadBase::ThreadBase;
    protected:
        // Thread function
        virtual void _function(std::string filepath);

    private:
        RGBA32::Pixels _pixels;
        int _w{ 0 };
        int _h{ 0 };
    } _loading_thread;
    friend _LoadingThread;

    void _updatePlanesScaling();
};

__SSS_GL_END

bool SSS::GL::Texture2D::_LoadingThread::LOG::run_state{ true };