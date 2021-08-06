#pragma once

#include "_internal/pointers.hpp"

__SSS_GL_BEGIN

class Texture : public _internal::WindowObject {
    friend class Window;
    friend class Plane;

private:
    Texture(std::weak_ptr<Window> window);

public:
    ~Texture();

    struct LOG {
        static bool constructor;
        static bool destructor;
    };

    using Ptr = std::unique_ptr<Texture>;
    
    enum class Type {
        Raw,    // Raw editing
        File,   // File loading
        Text    // TextArea logic
    };

private:
    _internal::Texture _raw_texture;
    int _w{ 0 }, _h{ 0 };
    RGBA32::Pixels _pixels;
    TR::TextArea::Shared _text_area;
    Type _type{ Type::Raw };

public:
    void edit(void const* pixels, int width, int height);
    void useFile(std::string filepath);

    inline void setType(Type type) noexcept { _type = type; };
    inline Type getType() const noexcept { return _type; };

    void bind();

    inline void getDimensions(int& w, int& h) const noexcept { w = _w; h = _h; };
    inline TR::TextArea::Shared const& getTextArea() noexcept { return _text_area; };

private:

// --- Image loading ---

    // Loading thread which fills _raw_pixels using stb_image
    class _LoadingThread : public ThreadBase <std::string> {
        friend class Window;
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

bool SSS::GL::Texture::_LoadingThread::LOG::run_state{ true };