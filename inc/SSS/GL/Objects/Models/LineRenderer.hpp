#ifndef SSS_GL_LINERENDERER_HPP
#define SSS_GL_LINERENDERER_HPP

#include "SSS/GL/Objects/Models/Line.hpp"
#include "SSS/GL/Objects/Renderer.hpp"
#include "SSS/GL/Objects/Camera.hpp"

SSS_GL_BEGIN;

class LineRenderer : public Renderer {
    friend class Window;

private:
    LineRenderer(std::weak_ptr<Window> win, uint32_t id);

public:
    Camera::Shared camera;
    void render();

private:
    Basic::VAO::Ptr _vao;
    Basic::VBO::Ptr _vbo;
    Basic::IBO::Ptr _ibo;

    void gen_batch(Polyline::Vertex::Vec& mesh, Polyline::Indices::Vec& indices);
};

SSS_GL_END;

#endif // SSS_GL_LINERENDERER_HPP