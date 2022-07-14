#pragma once

#include "Line.hpp"
#include "../Renderer.hpp"

SSS_GL_BEGIN;

class LineRenderer : public Renderer {
    friend class Window;

private:
    LineRenderer(std::weak_ptr<Window> win, uint32_t id);

public:
    uint32_t cam_id{ 0 };
    void render();

private:
    Basic::VAO::Ptr _vao;
    Basic::VBO::Ptr _vbo;
    Basic::IBO::Ptr _ibo;

    void gen_batch(Polyline::Vertex::Vec& mesh, Polyline::Indices::Vec& indices);
};

SSS_GL_END;