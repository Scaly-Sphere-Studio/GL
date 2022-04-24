#pragma once

#include "../Renderer.hpp"

__SSS_GL_BEGIN;
__INTERNAL_BEGIN;

class PlaneRenderer : public Renderer {
    friend class Window;

private:
    static constexpr uint32_t glsl_max_array_size = 128;

    PlaneRenderer(std::weak_ptr<Window> window);

    using Mat4_array = std::array<glm::mat4, glsl_max_array_size>;
    void _renderPart(uint32_t& count, bool reset_depth) const;

public:
    virtual void render();

private:
    bool _findNearestModel(float x, float y);
    uint32_t _hovered_id{ 0 };
    double _hovered_z{ DBL_MAX };
    Mat4_array _VPs;
    Mat4_array _Models;
};

__INTERNAL_END;
__SSS_GL_END;