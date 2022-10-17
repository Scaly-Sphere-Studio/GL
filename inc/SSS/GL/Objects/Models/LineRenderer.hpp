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
    virtual void render();
    static inline LineRenderer& create() {
        return Renderer::create<LineRenderer>();
    };
    static inline LineRenderer& create(Window::Shared win) {
        return Renderer::create<LineRenderer>(win);
    };

private:
    Basic::VAO _vao;
    Basic::VBO _vbo;
    Basic::IBO _ibo;

    void gen_batch(Polyline::Vertex::Vec& mesh, Polyline::Indices::Vec& indices);
};

SSS_GL_END;

#endif // SSS_GL_LINERENDERER_HPP