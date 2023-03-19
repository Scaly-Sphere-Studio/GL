#ifndef SSS_GL_PLANERENDERER_HPP
#define SSS_GL_PLANERENDERER_HPP

#include "SSS/GL/Objects/Models/Plane.hpp"
#include "SSS/GL/Objects/Renderer.hpp"
#include "SSS/GL/Objects/Camera.hpp"

/** @file
 *  Defines internal class behind SSS::GL::Plane::Renderer alias.
 */

SSS_GL_BEGIN;

/** Dedicated Renderer for Plane instances.
 *  Specify a chunk of objects to be rendered.
 *  This is useful to enforce specific orders of chunks without
 *  having to worry about their depth (Background, Scene, Text, UI...).
 */
class PlaneRenderer final : public Renderer<PlaneRenderer> {
    friend class Basic::SharedBase<PlaneRenderer>;
    friend class Window;

private:
    PlaneRenderer(std::shared_ptr<Window> window);

    void _renderPart(Shaders& shader, uint32_t& count) const;

public:
    void render() override;

    using Renderer::create;
    static Shared create(Camera::Shared cam, bool clear_depth_buffer = false);

    /** Whether to reset Z-buffer before rendering.*/
    bool clear_depth_buffer{ false };
    /** Specified Camera.*/
    Camera::Shared camera;
    /** Specified Planes.*/
    std::vector<Plane::Shared> planes;

private:
    Basic::VAO _vao;
    Basic::VBO _vbo;
    Basic::IBO _ibo;
    
    std::vector<glm::mat4> _VPs;
    std::vector<glm::mat4> _Models;
    std::vector<float> _TextureOffsets;
    std::vector<float> _Alphas;

    Plane::Weak _hovered;
    double _hovered_z{ DBL_MAX };
    bool _findNearestModel(float x, float y);
};

SSS_GL_END;

#endif // SSS_GL_PLANERENDERER_HPP