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
        Raw,    // Raw editing & file loading
        Text    // TextArea logic
    };

private:
    _internal::Texture _raw_texture;
    int _raw_w{ 0 }, _raw_h{ 0 };
    int _text_w{ 0 }, _text_h{ 0 };
    RGBA32::Pixels _pixels;
    TR::TextArea::Shared _text_area;
    Type _type{ Type::Raw };

public:
    void edit(void const* pixels, int width, int height);
    void useFile(std::string filepath);

    void setType(Type type) noexcept;
    inline Type getType() const noexcept { return _type; };

    inline void bind() const { _raw_texture.bind(); };

    void setTextArea(TR::TextArea::Shared text_area);
    inline TR::TextArea::Shared const& getTextArea() const noexcept { return _text_area; };
    
    void getDimensions(int& w, int& h) const noexcept;
    GLuint getTexID() const noexcept { return _raw_texture.id; };

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
    void _internal_edit(void const* pixels, int w, int h);
};

__SSS_GL_END