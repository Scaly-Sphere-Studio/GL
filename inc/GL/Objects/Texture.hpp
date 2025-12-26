#ifndef SSS_GL_TEXTURE_HPP
#define SSS_GL_TEXTURE_HPP

#include <SSS/Text-Rendering.hpp>
#include <SSS/Commons/eventList.hpp>
#include "Basic.hpp"


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


INTERNAL_BEGIN;

struct APNGFrame {
    std::vector<uint8_t> vec;
    std::vector<uint8_t*> rows;
    unsigned int w{ 0 }, h{ 0 }, delay_num{ 0 }, delay_den{ 0 };
};
int load_apng(char const* filepath, std::vector<APNGFrame>& frames);

INTERNAL_END;


// Ignore warning about STL exports as they're private members
#pragma warning(push, 2)
#pragma warning(disable: 4251)
#pragma warning(disable: 4275)

/** Handles raw edits, image loading, and text rendering in an
 *  internal Basic::Texture.
 *  @sa Window::createTexture()
 */
class SSS_GL_API Texture : public Observer, public Subject, public InstancedClass<Texture>, public _EventRegistry<Texture> {
    friend SharedClass;
    friend _EventRegistry<Texture>;

private:
    Texture();

public:
    /** Destructor, can log but otherwise default.*/
    ~Texture();

    using InstancedClass::create;
    static Shared create(std::string const& filepath);
    static Shared create(TR::Area::Shared area);

    /** The Texture type, mainly to know which pixels to use (internal or TR).
     *  @sa setType(), getType()
     */
    enum class Type {
        /** Image loading and raw edits are used*/
        Raw,
        /** SSS::TR::Area is used*/
        Text
    };

    struct Frame {
        // Pixel array
        RGBA32::Vector pixels;
        // Delay, in ns for precision. 40ms would be 25FPS
        std::chrono::nanoseconds delay;
        // Vector
        class Vector : public std::vector<Frame> {
        public:
            using vector::vector;
            std::chrono::nanoseconds total_time;
            int w{ 0 };
            int h{ 0 };
        };
    };

    inline static void setResourceFolder(std::string const& path) { _resource_folder = path; };
    inline static std::string getResourceFolder() { return _resource_folder; };

private:
    static std::string _resource_folder;

    //static Basic::Texture 
    Basic::Texture _raw_texture;    // OpenGL texture
    Type _type{ Type::Raw };        // Texture type
    Frame::Vector _frames{ 1 };     // Vector of frames (is used for images AND animations)
    TR::Area::Shared _area;         // TR::Area
    std::string _filepath;          // Image filepath
    std::function<void(Texture&)> _callback_f;

public:
    inline void setUpdateCallback(std::function<void(Texture&)> f) noexcept { _callback_f = f; };

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
    inline std::string getFilepath() const noexcept { return _filepath; };
    /** Edits the raw pixels of this instance.
     *  @sa getRawPixels, getRawDimensions()
     */
    void editRawPixels(void const* pixels, int width, int height);

    void setColor(RGBA32 color);

    inline auto const& getRawPixels(size_t id = 0) const noexcept { return _frames.at(id).pixels; };

    inline auto const& getFrames() const noexcept { return _frames; };

    /** Sets the TR::Area to be used when type is set to Type::Text.
     *  As for loadImage(), TR::Area work asynchronously. Whenever the
     *  corresponding TR::Area's text rendering is completed, the next
     *  call to pollEverything() will update every Texture instance
     *  linked to it.
     *  @sa getTextArea()
     */
    inline void setTextArea(TR::Area::Shared area);
    /** Returns the TR::Area corresponding to the ID set via
     *  setTextAreaID(), or nullptr.
     *  @sa getTextAreaID()
     */
    inline TR::Area::Shared getTextArea() const noexcept { return _area; };
    
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
    std::tuple<int, int> getCurrentDimensions() const noexcept;

    void savePNG() const;

private:

    virtual void _subjectUpdate(Subject const& subject, int event_id) override;

    // Async class which fills _raw_pixels using stb_image
    class _AsyncLoading : public Async<std::string, std::string> {
        friend Texture;
    protected:
        virtual void _asyncFunction(std::string folder, std::string filepath);
    private:
        Frame::Vector _frames;
    } _loading_thread;

    // Simple internal edit based on set type
    void _internalEdit(Type type);

    static void _register();
};

#pragma warning(pop)

SSS_GL_END;

#endif // SSS_GL_TEXTURE_HPP