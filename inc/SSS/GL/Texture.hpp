#pragma once

#include "Basics.hpp"

/** @file
 *  Defines class SSS::GL::Texture.
 */

SSS_GL_BEGIN;

class Texture : public _internal::WindowObject {

    friend void pollEverything();
    friend class Window;
    friend class Plane;

private:
    Texture(std::weak_ptr<Window> window);

public:
    ~Texture();

    /** \cond TODO*/
    struct LOG {
        static bool constructor;
        static bool destructor;
    };
    /** \endcond*/

    using Ptr = std::unique_ptr<Texture>;
    
    enum class Type {
        Raw,    // Raw editing & file loading
        Text    // TextArea logic
    };

private:
    Basic::Texture _raw_texture;
    int _raw_w{ 0 }, _raw_h{ 0 };
    int _text_w{ 0 }, _text_h{ 0 };
    RGBA32::Vector _pixels;
    uint32_t _text_area_id{ 0 };
    Type _type{ Type::Raw };

public:
    void edit(void const* pixels, int width, int height);
    void useFile(std::string filepath);

    void setType(Type type) noexcept;
    inline Type getType() const noexcept { return _type; };

    inline void bind() const { _raw_texture.bind(); };

    void setTextAreaID(uint32_t id);
    inline uint32_t getTextAreaID() const noexcept { return _text_area_id; };
    TR::Area::Ptr const& getTextArea() const noexcept;
    
    void getDimensions(int& w, int& h) const noexcept;
    GLuint getTexID() const noexcept { return _raw_texture.id; };

private:

// --- Image loading ---

    // Async class which fills _raw_pixels using stb_image
    class _AsyncLoading : public AsyncBase <std::string> {
        friend void pollEverything();
    protected:
        virtual void _asyncFunction(std::string filepath);
    private:
        RGBA32::Vector _pixels;
        int _w{ 0 };
        int _h{ 0 };
    } _loading_thread;
    friend _AsyncLoading;

    void _updatePlanesScaling();
    void _internal_edit(void const* pixels, int w, int h);
};

SSS_GL_END;