#pragma once

#include "SSS/GL/Objects/Renderer.hpp"

/** @file
 *  Defines internal class behind SSS::GL::Plane::Renderer alias.
 */

SSS_GL_BEGIN;
INTERNAL_BEGIN;

class PlaneRenderer : public Renderer {
    friend class Window;

private:
    static constexpr uint32_t glsl_max_array_size = 128;

    PlaneRenderer(std::weak_ptr<Window> window, uint32_t id);

    using Mat4_array = std::array<glm::mat4, glsl_max_array_size>;
    void _renderPart(Shaders::Ptr const& shader,
        uint32_t& count, bool reset_depth) const;

public:
    virtual void render();

private:
    Basic::VAO::Ptr _vao;
    Basic::VBO::Ptr _vbo;
    Basic::IBO::Ptr _ibo;
    
    Mat4_array _VPs{ };
    Mat4_array _Models{ };

    uint32_t _hovered_id{ 0 };
    double _hovered_z{ DBL_MAX };
    bool _findNearestModel(float x, float y);
};

INTERNAL_END;
SSS_GL_END;