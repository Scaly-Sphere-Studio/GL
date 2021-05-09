#pragma once

#include "_internal/pointers.hpp"

__SSS_GL_BEGIN

class TextureBase {
public:
    using Shared = std::shared_ptr<TextureBase>;
protected:
    TextureBase(GLenum target) : _raw_texture(target) {};
public:
    TextureBase() = delete;
    virtual void bind() { _raw_texture.bind(); };
    inline void getDimensions(int& w, int& h) { w = _w; h = _h; };
    inline RGBA32::Pixels const& getPixels() { return _pixels; };
protected:
    int _w{ 0 }, _h{ 0 };
    _internal::Texture _raw_texture;
    RGBA32::Pixels _pixels;
};

class Texture2D : public TextureBase, public std::enable_shared_from_this<Texture2D> {
    friend class Plane;

protected:
    Texture2D();
public:
    ~Texture2D();

    struct LOG {
        static bool constructor;
        static bool destructor;
    };

    using Shared = std::shared_ptr<Texture2D>;
    using Ptr = std::unique_ptr<Texture2D>;

    static Shared create();
    static Shared create(std::string const& filepath);
    static Shared create(void const* pixels, int width, int height);

    static void pollThreads();

protected:
    using Weak = std::weak_ptr<Texture2D>;
    static std::deque<Weak> _instances;

public:
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
};

__SSS_GL_END

bool SSS::GL::Texture2D::_LoadingThread::LOG::run_state{ true };