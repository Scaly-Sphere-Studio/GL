#include "SSS/GL/Objects/Models/LineRenderer.hpp"
#include "SSS/GL/Window.hpp"

SSS_GL_BEGIN;

LineRenderer::LineRenderer(std::shared_ptr<Window> win)
    : Renderer(win), _vao(win), _vbo(win), _ibo(win)
{
    _vao.bind();

    _vbo.bind();
    //Coordinates
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2,
        GL_FLOAT, GL_FALSE,
        sizeof(Polyline::Vertex), (void*)0);
    //Colors
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4,
        GL_FLOAT, GL_FALSE,
        sizeof(Polyline::Vertex),
        (void*)(sizeof(glm::vec3)));

    _ibo.bind();
    
    _vao.unbind();
}

void LineRenderer::gen_batch(Polyline::Vertex::Vec& mesh, Polyline::Indices::Vec& indices)
{
    size_t indice_size = 0, mesh_size = 0, offset = 0;

    std::sort(Polyline::_batch.begin(), Polyline::_batch.end(), Polyline::sort);

    //Check if the lines pointer are expired and set the necessary reserve size for the batch
    for (std::weak_ptr<Polyline> arrow_weak : Polyline::_batch) {

        Polyline::Shared const arrow = arrow_weak.lock();
        if (!arrow) {
            continue;
        }

        indice_size += arrow->indices.size();
        mesh_size += arrow->mesh.size();
    }

    indices.reserve(indice_size);
    mesh.reserve(mesh_size);


    Polyline::Indices tmp;

    //Add the vertices and indices in the batch    
    for (std::weak_ptr<Polyline> const& arrow_weak : Polyline::_batch) {
        
        Polyline::Shared const arrow = arrow_weak.lock();
        if (!arrow) {
            continue;
        }

        //Create the batch indices
        for (Polyline::Indices const& t : arrow->indices) {
            tmp = t;
            tmp._bc += static_cast<uint32_t>(offset);
            tmp._sc += static_cast<uint32_t>(offset);
            tmp._uc += static_cast<uint32_t>(offset);

            indices.emplace_back(tmp);
        }

        offset += arrow->mesh.size();

        //batch the vertices
        mesh.insert(mesh.end(), arrow->mesh.begin(), arrow->mesh.end());
    }

}

void LineRenderer::render()
{
    static size_t size;

    Context const context = getContext();

    _vao.bind();

    if (Polyline::modified) {

        Polyline::Vertex::Vec tmp_v;
        Polyline::Indices::Vec tmp_i;

        gen_batch(tmp_v, tmp_i);

        _vbo.edit(
            tmp_v.size() * sizeof(Polyline::Vertex),
            tmp_v.data(),
            GL_STATIC_DRAW
        );

        _ibo.edit(
            tmp_i.size() * sizeof(Polyline::Indices),
            tmp_i.data(),
            GL_STATIC_DRAW);

        size = tmp_i.size();

        tmp_v.clear();
        tmp_i.clear();

        Polyline::modified = false;
    }


    Shaders::Shared shader = getShaders();
    if (!shader) {
        LOG_METHOD_WRN("No shaders bound");
        return;
    }
    glm::mat4 const mvp = camera ? camera->getVP() : glm::mat4(1);
    shader->use();
    shader->setUniformMat4fv("u_MVP", 1, GL_FALSE, &mvp[0][0]);

    glDrawElements(GL_TRIANGLES, 3 * static_cast<GLsizei>(size), GL_UNSIGNED_INT, (void*)0);

    _vao.unbind();
}

SSS_GL_END;