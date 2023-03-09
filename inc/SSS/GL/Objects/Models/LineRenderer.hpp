#ifndef SSS_GL_LINERENDERER_HPP
#define SSS_GL_LINERENDERER_HPP

#include "SSS/GL/Objects/Models/Line.hpp"
#include "SSS/GL/Objects/Renderer.hpp"
#include "SSS/GL/Objects/Camera.hpp"

SSS_GL_BEGIN;

class LineRenderer : public Renderer<LineRenderer> {
    friend class Basic::SharedBase<LineRenderer>;
    friend class Window;

private:
    LineRenderer(std::shared_ptr<Window> win);

public:
    Camera::Shared camera;
    void render() override;

private:
    Basic::VAO _vao;
    Basic::VBO _vbo;
    Basic::IBO _ibo;

    void gen_batch(Polyline::Vertex::Vec& mesh, Polyline::Indices::Vec& indices);
};

SSS_GL_END;

#endif // SSS_GL_LINERENDERER_HPP