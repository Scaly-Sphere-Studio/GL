#ifndef SSS_GL_PLANERENDERER_HPP
#define SSS_GL_PLANERENDERER_HPP

#include "SSS/GL/Objects/Models/Plane.hpp"
#include "SSS/GL/Objects/Renderer.hpp"
#include "SSS/GL/Objects/Camera.hpp"

/** @file
 *  Defines internal class behind SSS::GL::Plane::Renderer alias.
 */

SSS_GL_BEGIN;

/** Dedicated Renderer for Plane instances.*/
class PlaneRenderer final : public Renderer {
    friend class Window;

private:
    PlaneRenderer(std::weak_ptr<Window> window, uint32_t id);

    using Mat4_array = std::vector<glm::mat4>;
    void _renderPart(Shaders::Ptr const& shader,
        uint32_t& count, bool reset_depth) const;

public:
    virtual void render();

    /** Specify a chunk of objects to be rendered.
     *  This is useful to enforce specific orders of chunks without
     *  having to worry about their depth (Background, Scene, Text, UI...).\n
     *  Stored in Renderer::chunks.
     */
    struct Chunk final {
        Chunk(Camera::Shared cam = nullptr, bool reset_depth = false)
            : camera(cam), reset_depth_before(reset_depth) {};
        /** Optional title for UI purpose only.*/
        std::string title;
        /** Wether to clear the depth buffer before rendering this chunk,
         *  so that future objects will always be on top of previously
         *  rendered stuff.
         */
        bool reset_depth_before{ false };
        /** Specified Camera.*/
        Camera::Shared camera;
        /** Specified Planes.*/
        std::vector<Plane::Shared> planes;
    };

    /** Vector of Chunk instances, which will be rendered one by one.*/
    std::vector<Chunk> chunks;

private:
    Basic::VAO::Ptr _vao;
    Basic::VBO::Ptr _vbo;
    Basic::IBO::Ptr _ibo;
    
    Mat4_array _VPs;
    Mat4_array _Models;

    Plane::Weak _hovered;
    double _hovered_z{ DBL_MAX };
    bool _findNearestModel(float x, float y);
};

SSS_GL_END;

#endif // SSS_GL_PLANERENDERER_HPP