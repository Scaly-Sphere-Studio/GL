#include "GL/Objects/Models/LineRenderer.hpp"
#include "GL/Window.hpp"

SSS_GL_BEGIN;

LineRenderer::LineRenderer()
{
    auto shader = Window::getPresetShaders(static_cast<uint32_t>(Shaders::Preset::Line));
    addMaterial("default", Material(shader));
}

LineRenderer::Entry::Entry(Polyline::Shared line)
    : line(std::move(line))
{
    vao.setup([this]() {
        vbo.bind();
        ibo.bind();
        //Coordinates
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3,
            GL_FLOAT, GL_FALSE,
            sizeof(Polyline::Vertex), (void*)0);
        //Colors
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4,
            GL_FLOAT, GL_FALSE,
            sizeof(Polyline::Vertex),
            (void*)(sizeof(glm::vec3)));
    });
    vao.unbind();
}

void LineRenderer::addLine(Polyline::Shared line)
{
    if (!line)
        return;
    for (auto const& e : _lines) {
        if (e->line == line)
            return;
    }
    _lines.emplace_back(std::make_unique<Entry>(std::move(line)));
}

void LineRenderer::removeLine(Polyline::Shared const& line)
{
    _lines.erase(
        std::remove_if(_lines.begin(), _lines.end(),
            [&](std::unique_ptr<Entry> const& e) { return e->line == line; }),
        _lines.end());
}

void LineRenderer::render()
{
    if (!isActive() || _lines.empty()) {
        return;
    }

    Material& mat = swapMaterial("default");
    mat.set("u_VP", camera ? camera->getVP() : glm::mat4(1));

    for (auto const& e : _lines) {
        Polyline& line = *e->line;
        if (line.indices.empty())
            continue;

        e->vao.bind();

        // Upload only if the mesh has been regenerated
        if (!e->uploaded || e->uploaded_version != line.getMeshVersion()) {
            e->vbo.edit(
                line.mesh.size() * sizeof(Polyline::Vertex),
                line.mesh.data(),
                GL_DYNAMIC_DRAW);
            e->ibo.edit(
                line.indices.size() * sizeof(Polyline::Indices),
                line.indices.data(),
                GL_DYNAMIC_DRAW);
            e->uploaded_version = line.getMeshVersion();
            e->uploaded = true;
        }

        mat.set("u_Model", line.getModelMat4());
        glDrawElements(GL_TRIANGLES, 3 * static_cast<GLsizei>(line.indices.size()),
            GL_UNSIGNED_INT, (void*)0);
    }

    glBindVertexArray(0);
}

SSS_GL_END;
