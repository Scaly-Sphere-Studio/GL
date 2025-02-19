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
class SSS_GL_API PlaneRenderer : public Renderer<PlaneRenderer> {
    friend class SharedClass;
    friend class Window;

private:
    PlaneRenderer();
    void _renderPart(Shaders& shader, uint32_t& count, uint32_t& offset) const;
    bool _containsOneOf(PlaneBase::RawSet const& set) const noexcept;

    template <typename T>
    void _updateVBO(T(PlaneBase::* getMember)() const, Basic::VBO& vbo);

public:
    virtual void render() override;

    /** Whether to reset Z-buffer before rendering.*/
    bool clear_depth_buffer{ false };
    /** Specified Camera.*/
    Camera::Shared camera;
    /** Specified Planes.*/

private:
    std::vector<std::shared_ptr<PlaneBase>> _planes;
public:
    inline auto const& getPlanes() const noexcept { return _planes; };

    template <std::derived_from<PlaneBase> _Plane>
    void setPlanes(std::vector<std::shared_ptr<_Plane>> planes)
    {
        _planes.clear();
        addPlanes(planes);
    }
    
    void addPlane(std::shared_ptr<PlaneBase> plane);
    void removePlane(std::shared_ptr<PlaneBase> plane);
    
    template <std::derived_from<PlaneBase> _Plane>
    void addPlanes(std::vector<std::shared_ptr<_Plane>> planes)
    {
        _planes.insert(_planes.end(), planes.cbegin(), planes.cend());
        _update_vbos = true;
    }
    
    template <std::derived_from<PlaneBase> _Plane>
    void removePlanes(std::vector<std::shared_ptr<_Plane>> planes)
    {
        _planes.erase(
            std::remove_if(
                _planes.begin(),
                _planes.end(),
                [planes](auto p1) {
                    return std::any_of(
                        planes.cbegin(),
                        planes.cend(),
                        [p1](auto p2) { return p1.get() == p2.get(); }
                    );
                }
            ), _planes.end()
        );
        _update_vbos = true;
    }

    using SharedClass::create;

    static auto create(Camera::Shared cam, bool clear_depth_buffer = false) {
        auto shared = SharedClass::create();
        shared->camera = cam;
        shared->clear_depth_buffer = clear_depth_buffer;
        return shared;
    }

    template<std::derived_from<PlaneBase> _Plane, class _T>
    _T forEach(std::function<_T (_Plane&)> func)
    {
        _T ret{};
        for (auto const& plane : _planes) {
            if (!plane) continue;
            auto cast_plane = std::dynamic_pointer_cast<_Plane>(plane);
            ret += func(*cast_plane);
        }
        return ret;
    };

    template<std::derived_from<PlaneBase> _Plane>
    void forEach(std::function<void (_Plane&)> func)
    {
        for (auto const& plane : _planes) {
            if (!plane) continue;
            auto cast_plane = std::dynamic_pointer_cast<_Plane>(plane);
            func(*cast_plane);
        }
    };

    template<std::derived_from<PlaneBase> _Plane>
    bool forEach(std::function<bool (_Plane&)> func)
    {
        for (auto const& plane : _planes) {
            if (!plane) continue;
            auto cast_plane = std::dynamic_pointer_cast<_Plane>(plane);
            if (func(*cast_plane))
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

    // To update all dynamic vbos
    bool _update_vbos{ true };

    std::weak_ptr<PlaneBase> _hovered;
    double _hovered_z{ DBL_MAX };
    bool _findNearestModel(double x, double y);
};

#pragma warning(pop)

SSS_GL_END;

#endif // SSS_GL_PLANERENDERER_HPP