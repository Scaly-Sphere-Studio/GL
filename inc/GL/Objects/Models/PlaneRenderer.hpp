#ifndef SSS_GL_PLANERENDERER_HPP
#define SSS_GL_PLANERENDERER_HPP

#include "Plane.hpp"
#include "../Renderer.hpp"
#include "../Camera.hpp"

/** @file
 *  Defines internal class behind SSS::GL::Plane::Renderer alias.
 */

SSS_GL_BEGIN;

// Ignore warning about STL exports as they're private members
#pragma warning(push, 2)
#pragma warning(disable: 4251)
#pragma warning(disable: 4275)

/** Dedicated Renderer for Plane instances.
 *  Specify a chunk of objects to be rendered.
 *  This is useful to enforce specific orders of chunks without
 *  having to worry about their depth (Background, Scene, Text, UI...).
 */
class SSS_GL_API PlaneRendererBase : public RendererBase {
    friend class Window;

protected:
    PlaneRendererBase();
private:
    void _renderPart(Shaders& shader, uint32_t& count) const;

public:
    void render() override;

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

template <class Derived>
class PlaneRendererTemplate : public PlaneRendererBase, public Basic::InstancedBase<Derived> {
protected:
    PlaneRendererTemplate() = default;
public:
    static auto create(Camera::Shared cam, bool clear_depth_buffer = false) {
        auto shared = Renderer<Derived>::create();
        shared->camera = cam;
        shared->clear_depth_buffer = clear_depth_buffer;
        return shared;
    }

    using Basic::SharedBase<Derived>::Shared;
    using Basic::InstancedBase<Derived>::create;
    virtual RendererBase::Shared getShared() noexcept override {
        Shared shared = Basic::SharedBase<Derived>::shared_from_this();
        return shared;
    };
};

class PlaneRenderer : public PlaneRendererTemplate<PlaneRenderer> {
    friend class Basic::SharedBase<PlaneRenderer>;
private:
    PlaneRenderer() = default;
};

#pragma warning(pop)

SSS_GL_END;

#endif // SSS_GL_PLANERENDERER_HPP