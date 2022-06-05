#pragma once

#include <SSS/Text-Rendering.hpp>
#include "SSS/GL/Objects/Basic.hpp"

/** @file
 *  Defines class SSS::GL::Texture.
 */

namespace SSS::Log::GL {
    /** Logging properties for SSS::GL::Texture.*/
    struct Texture : public LogBase<Texture> {
        using LOG_STRUCT_BASICS(Log, Texture);
        bool life_state = false;
        bool edit = false;
    };
}

SSS_GL_BEGIN;

/** Handles raw edits, image loading, and text rendering in an
 *  internal Basic::Texture.
 *  @sa Window::createTexture()
 */
class Texture final : public _internal::WindowObjectWithID {

    friend bool pollEverything();
    friend class Window;

private:
    Texture(std::weak_ptr<Window> window, uint32_t id);

public:
    /** Destructor, can log but otherwise default.*/
    ~Texture();

    /** Unique ptr stored in Window objects.*/
    using Ptr = std::unique_ptr<Texture>;
    
    /** The Texture type, mainly to know which pixels to use (internal or TR).
     *  @sa setType(), getType()
     */
    enum class Type {
        /** Image loading and raw edits are used*/
        Raw,
        /** SSS::TR::Area is used*/
        Text
    };

private:
    Basic::Texture _raw_texture;    // OpenGL texture
    Type _type{ Type::Raw };        // Texture type
    RGBA32::Vector _pixels;         // Raw pixels
    int _raw_w{ 0 }, _raw_h{ 0 };   // Raw dimensions
    int _text_w{ 0 }, _text_h{ 0 }; // Last TR dimensions (stored for scaling update)
    uint32_t _text_area_id{ 0 };    // TR::Area::Ptr ID inside TR::Area::Map

public:
    /** Explicitly sets the Texture::Type.
     *  @sa getType()
     */
    void setType(Type type) noexcept;
    /** Returns the current Texture::Type.
     *  @sa setType()
     */
    inline Type getType() const noexcept { return _type; };

    /** Asynchronously loads the image at given path (can be relative or absolute).
     *  Whenever the loading is completed, the next call to pollEverything() will
     *  update the raw pixels of this instance with retrieved data.
     *  @sa getRawPixels, getRawDimensions()
     */
    void loadImage(std::string const& filepath);
    /** Edits the raw pixels of this instance.
     *  @sa getRawPixels, getRawDimensions()
     */
    void editRawPixels(void const* pixels, int width, int height);
    /** Returns the raw pixels of this instance.
     *  @sa loadImage(), editRawPixels(), getRawDimensions()
     */
    inline RGBA32::Vector const& getRawPixels() const noexcept { return _pixels; };
    /** Copies the internal raw pixels' dimensions in given parameters.
     *  @sa loadImage(), editRawPixels(), getRawPixels()
     */
    inline void getRawDimensions(int& w, int& h) const noexcept { w = _raw_w; h = _raw_h; };

    /** Sets the TR::Area to be used when type is set to Type::Text.
     *  As for loadImage(), TR::Area work asynchronously. Whenever the
     *  corresponding TR::Area's text rendering is completed, the next
     *  call to pollEverything() will update every Texture instance
     *  linked to it.
     *  @sa getTextAreaID(), getTextArea()
     */
    void setTextAreaID(uint32_t id);
    /** Returns the TR::Area ID previously set via setTextAreaID(), or default (0).
     *  @sa getTextArea()
     */
    inline uint32_t getTextAreaID() const noexcept { return _text_area_id; };
    /** Returns the TR::Area::Ptr corresponding to the ID set via
     *  setTextAreaID(), or an empty ptr.
     *  @sa getTextAreaID()
     */
    TR::Area::Ptr const& getTextArea() const noexcept;
    
    /** Binds the internal Basic::Texture to the content in which it was created.
     *  Effectively calls Basic::Texture::bind().
     */
    inline void bind() const { _raw_texture.bind(); };
    /** Returns the internal Basic::Texture's ID.
     *  If you wish to %bind the internal texture, call bind().
     */
    GLuint getBasicTextureID() const noexcept { return _raw_texture.id; };
    /** Copies the internal Basic::Texture's current dimensions in given parameters.*/
    void getCurrentDimensions(int& w, int& h) const noexcept;

private:

    // Async class which fills _raw_pixels using stb_image
    class _AsyncLoading : public AsyncBase <std::string> {
        friend bool pollEverything();
    protected:
        virtual void _asyncFunction(std::string filepath);
    private:
        RGBA32::Vector _pixels;
        int _w{ 0 };
        int _h{ 0 };
    } _loading_thread;

    // Update the texture scaling of all Planes using this texture
    void _updatePlanesScaling();
    // Simple internal edit based on set type
    void _internalEdit(void const* pixels, int w, int h);
};

SSS_GL_END;