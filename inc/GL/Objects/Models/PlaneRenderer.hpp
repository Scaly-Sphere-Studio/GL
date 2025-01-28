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
class SSS_GL_API PlaneRenderer : public RendererBase, public Basic::InstancedBase<PlaneRenderer> {
    friend class Window;
    friend class Basic::SharedBase<PlaneRenderer>;

private:
    PlaneRenderer();
    void _renderPart(Shaders& shader, uint32_t& count) const;

public:
    void render() override;

    /** Whether to reset Z-buffer before rendering.*/
    bool clear_depth_buffer{ false };
    /** Specified Camera.*/
    Camera::Shared camera;
    /** Specified Planes.*/
    std::vector<Plane::Shared> planes;

    static auto create(Camera::Shared cam, bool clear_depth_buffer = false) {
        auto shared = Renderer<PlaneRenderer>::create();
        shared->camera = cam;
        shared->clear_depth_buffer = clear_depth_buffer;
        return shared;
    }

    using Basic::SharedBase<PlaneRenderer>::Shared;
    using Basic::InstancedBase<PlaneRenderer>::create;
    virtual RendererBase::Shared getShared() noexcept override {
        Shared shared = Basic::SharedBase<PlaneRenderer>::shared_from_this();
        return shared;
    };

    template <typename T>
    T forEach(std::function<T(Plane&)> func)
    {
        T ret{};
        for (auto const& plane : planes) {
            if (!plane)
                continue;
            ret += func(*plane);
        }
        return ret;
    };

    template<>
    void forEach(std::function<void(Plane&)> func)
    {
        for (auto const& plane : planes) {
            if (!plane)
                continue;
            func(*plane);
        }
    };

    template<>
    bool forEach(std::function<bool(Plane&)> func)
    {
        for (auto const& plane : planes) {
            if (!plane)
                continue;
            if (func(*plane))
                return true;
        }
        return false;
    };

private:
    Basic::VAO _vao;
    // Static Plane vertices
    Basic::VBO _static_vbo;
    Basic::IBO _static_ibo;
    // Plane Model mat4
    Basic::VBO _model_vbo;
    // Plane alpha
    Basic::VBO _alpha_vbo;
    // Plane texture offset (used to read apng)
    Basic::VBO _tex_offset_vbo;

    Plane::Weak _hovered;
    double _hovered_z{ DBL_MAX };
    bool _findNearestModel(double x, double y);
};

#pragma warning(pop)

SSS_GL_END;

#endif // SSS_GL_PLANERENDERER_HPP