#pragma once

#include "Shaders.hpp"

/** @file
 *  Defines abstract class SSS::GL::Renderer.
 */

SSS_GL_BEGIN;

/** Abstract class for specialized rendering logic.
 *  @sa Window::createRenderer(), Plane::Renderer
 */
class Renderer : public _internal::WindowObjectWithID {
protected:
    /** Constructor, ensures the renderer is bound to a Window instance.*/
    Renderer(std::weak_ptr<Window> window, uint32_t id)
        : _internal::WindowObjectWithID(window, id) {};
public:
    /** Virtual destructor, default.*/
    virtual ~Renderer()                     = default;  // Destructor
    /** \cond INTERNAL*/
    Renderer()                              = delete;   // Constructor (default)
    Renderer(const Renderer&)               = delete;   // Copy constructor
    Renderer(Renderer&&)                    = delete;   // Move constructor
    Renderer& operator=(const Renderer&)    = delete;   // Copy assignment
    Renderer& operator=(Renderer&&)         = delete;   // Move assignment
    /** \endcond*/
    
    /** Unique ptr stored in Window objects.*/
    using Ptr = std::unique_ptr<Renderer>;

    /** Specify a chunk of objects to be rendered.
     *  This is useful to enforce specific orders of chunks without
     *  having to worry about their depth (Background, Scene, Text, UI...).\n
     *  Stored in Renderer::chunks.
     */
    struct Chunk {
        /** Optional title for UI purpose only.*/
        std::string title;
        /** Wether to clear the depth buffer before rendering this chunk,
         *  so that future objects will always be on top of previously
         *  rendered stuff.
         */
        bool reset_depth_before{ false };
        /** Wether this chunk should use the specified Camera in the MVP matrices.*/
        bool use_camera{ true };
        /** Specified Camera ID for the MVP matrices.*/
        uint32_t camera_ID{ 0 };
        /** Specified object IDs.*/
        std::deque<uint32_t> objects;
    };

    /** Deque of Chunk instances, which will be rendered one by one.*/
    std::deque<Chunk> chunks;
    /** Optional title for UI purpose only.*/
    std::string title;

    /** Your rendering logic here.*/
    virtual void render() = 0;

private:
    uint32_t _shaders_id{ 0 };
public:
    /** Sets the Shaders ID to be used in render().*/
    inline void setShadersID(uint32_t id) noexcept { _shaders_id = id; };
    /** Retrieves the set Shaders ID to be used in render().*/
    inline uint32_t getShadersID() const noexcept { return _shaders_id; };

private:
    bool _is_active{ true };
public:
    /** Enables or disables the renderer.*/
    inline void setActivity(bool state) noexcept { _is_active = state; };
    /** Whether the renderer is enabled or disabled.*/
    inline bool isActive() const noexcept { return _is_active; };
};

SSS_GL_END;