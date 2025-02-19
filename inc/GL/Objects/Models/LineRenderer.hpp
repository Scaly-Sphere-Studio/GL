#ifndef SSS_GL_LINERENDERER_HPP
#define SSS_GL_LINERENDERER_HPP

#include "Line.hpp"
#include "../Renderer.hpp"
#include "../Camera.hpp"

SSS_GL_BEGIN;

// Ignore warning about STL exports as they're private members
#pragma warning(push, 2)
#pragma warning(disable: 4251)
#pragma warning(disable: 4275)

class SSS_GL_API LineRenderer : public Renderer<LineRenderer> {
    friend class SharedClass;
    friend class Window;

private:
    LineRenderer();

public:
    Camera::Shared camera;
    virtual void render() override;

private:
    Basic::VAO _vao;
    Basic::VBO _vbo;
    Basic::IBO _ibo;

    void gen_batch(Polyline::Vertex::Vec& mesh, Polyline::Indices::Vec& indices);
};

#pragma warning(pop)

SSS_GL_END;

#endif // SSS_GL_LINERENDERER_HPP