#pragma once

#include "_internal/pointers.hpp"

__SSS_GL_BEGIN

class Window;

class TextureBase : public _internal::WindowObject {
public:
    using Shared = std::shared_ptr<TextureBase>;
protected:
    TextureBase(std::shared_ptr<Window> window, GLenum target)
        : _internal::WindowObject(window), _raw_texture(window, target) {};
public:
    TextureBase() = delete;
    virtual void bind() { _raw_texture.bind(); };
    inline void getDimensions(int& w, int& h) const noexcept { w = _tex_w; h = _tex_h; };
    inline RGBA32::Pixels const& getStoredPixels() { return _pixels; };
protected:
    int _tex_w{ 0 }, _tex_h{ 0 };
    _internal::Texture _raw_texture;
    RGBA32::Pixels _pixels;
};

class Texture2D : public TextureBase, public std::enable_shared_from_this<Texture2D> {
    friend class Plane;
    friend class Window;

protected:
    Texture2D(std::shared_ptr<Window> window);
    using Weak = std::weak_ptr<Texture2D>;

private:
    static std::vector<Weak> _instances;

public:
    ~Texture2D();

    struct LOG {
        static bool constructor;
        static bool destructor;
    };

    using Shared = std::shared_ptr<Texture2D>;
    static Shared create(std::shared_ptr<Window> window);
    static Shared create(std::shared_ptr<Window> window,
        std::string const& filepath);
    static Shared create(std::shared_ptr<Window> window,
        void const* pixels, int width, int height);

    static void pollThreads();

    void useFile(std::string filepath);
    void edit(void const* pixels, int width, int height);

private:
// --- Image loading ---

    // Loading thread which fills _raw_pixels using stb_image
    class _LoadingThread : public ThreadBase <Texture2D::Weak, std::string> {
        // Export constructors
        using ThreadBase::ThreadBase;
    protected:
        // Thread function
        virtual void _function(Texture2D::Weak texture, std::string filepath);
    } _loading_thread;
    friend _LoadingThread;

    void _updatePlanesScaling();
};

__SSS_GL_END

bool SSS::GL::Texture2D::_LoadingThread::LOG::run_state{ true };