#pragma once

#include "_internal/pointers.hpp"

__SSS_GL_BEGIN

class Texture2D : public std::enable_shared_from_this<Texture2D> {
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

    virtual inline void bind() {
        _raw_texture.bind();
    };
    // Returns stored pixels.
    // The vector is only filled if the texture was generated via a file.
    inline RGBA32::Pixels const& getPixels() const noexcept {
        return _pixels;
    };
    inline void getDimensions(int& width, int& height) const noexcept {
        width = _w;
        height = _h;
    };

private:
    int _w{ 0 }, _h{ 0 };
    _internal::Texture _raw_texture{ GL_TEXTURE_2D };

// --- Image loading ---

    // Vector of pixels
    RGBA32::Pixels _pixels;

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