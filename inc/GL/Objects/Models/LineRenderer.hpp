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

/** Renders a list of Polyline, one draw call per line.
 *  Each line keeps its own mesh and GPU buffers (uploaded only when the mesh
 *  changes), and is positioned through its own model matrix.
 */
class SSS_GL_API LineRenderer : public Renderer<LineRenderer> {
    friend class SharedClass;

private:
    LineRenderer();

public:
    Camera::Shared camera;
    virtual void render() override;

    void addLine(Polyline::Shared line);
    void removeLine(Polyline::Shared const& line);
    void clearLines() noexcept { _lines.clear(); };
    size_t getLineCount() const noexcept { return _lines.size(); };

private:
    // GPU data of a line, owned by the renderer
    struct Entry {
        Entry(Polyline::Shared line);

        Polyline::Shared line;
        Basic::VAO vao;
        Basic::VBO vbo;
        Basic::IBO ibo;
        uint32_t uploaded_version;
        bool uploaded{ false };
    };
    std::vector<std::unique_ptr<Entry>> _lines;
};

#pragma warning(pop)

SSS_GL_END;

#endif // SSS_GL_LINERENDERER_HPP
