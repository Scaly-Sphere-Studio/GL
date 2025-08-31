#include "GL/Objects/Models/PlaneRenderer.hpp"
#include "GL/Window.hpp"
#include <ranges>

SSS_GL_BEGIN;

PlaneRenderer::PlaneRenderer() try
{
    setShaders(Window::getPresetShaders(static_cast<uint32_t>(Shaders::Preset::Plane)));

    // Edit VBO
    constexpr float vertices[] = {
        // positions          // texture coords (1 - y)
        -0.5f,  0.5f, 0.0f,   0.f, 1.f - 1.f,   // top left
        -0.5f, -0.5f, 0.0f,   0.f, 1.f - 0.f,   // bottom left
         0.5f, -0.5f, 0.0f,   1.f, 1.f - 0.f,   // bottom right
         0.5f,  0.5f, 0.0f,   1.f, 1.f - 1.f,   // top right
    };
    _static_vbo.edit(sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Edit IBO
    constexpr unsigned int indices[] = {
        0, 1, 2,  // first triangle
        2, 3, 0   // second triangle
    };
    _static_ibo.edit(sizeof(indices), indices, GL_STATIC_DRAW);

    // Setup VAO
    _vao.setup([this]() {

        enum {
            PLANE_POS,
            PLANE_UV,

            PLANE_MODEL_MAT4_1,
            PLANE_MODEL_MAT4_2,
            PLANE_MODEL_MAT4_3,
            PLANE_MODEL_MAT4_4,

            PLANE_ALPHA,
            PLANE_TEX_OFFSET,
        };

        _static_vbo.bind();
        _static_ibo.bind();
        glEnableVertexAttribArray(PLANE_POS);
        glVertexAttribPointer(PLANE_POS, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(PLANE_UV);
        glVertexAttribPointer(PLANE_UV, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

        _model_vbo.bind();
        glEnableVertexAttribArray(PLANE_MODEL_MAT4_1);
        glEnableVertexAttribArray(PLANE_MODEL_MAT4_2);
        glEnableVertexAttribArray(PLANE_MODEL_MAT4_3);
        glEnableVertexAttribArray(PLANE_MODEL_MAT4_4);
        glVertexAttribPointer(PLANE_MODEL_MAT4_1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(0));
        glVertexAttribPointer(PLANE_MODEL_MAT4_2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(1 * sizeof(glm::vec4)));
        glVertexAttribPointer(PLANE_MODEL_MAT4_3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
        glVertexAttribPointer(PLANE_MODEL_MAT4_4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
        glVertexAttribDivisor(PLANE_MODEL_MAT4_1, 1);
        glVertexAttribDivisor(PLANE_MODEL_MAT4_2, 1);
        glVertexAttribDivisor(PLANE_MODEL_MAT4_3, 1);
        glVertexAttribDivisor(PLANE_MODEL_MAT4_4, 1);

        _alpha_vbo.bind();
        glEnableVertexAttribArray(PLANE_ALPHA);
        glVertexAttribPointer(PLANE_ALPHA, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
        glVertexAttribDivisor(PLANE_ALPHA, 1);

        _tex_offset_vbo.bind();
        glEnableVertexAttribArray(PLANE_TEX_OFFSET);
        glVertexAttribIPointer(PLANE_TEX_OFFSET, 1, GL_UNSIGNED_INT, sizeof(uint32_t), (void*)0);
        glVertexAttribDivisor(PLANE_TEX_OFFSET, 1);
        });
    _vao.unbind();
}
CATCH_AND_RETHROW_METHOD_EXC;

void PlaneRenderer::_renderPart(Shaders& shader, uint32_t& count, uint32_t& offset) const
{
    static constexpr std::array<GLint, 128> texture_IDs = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
        32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
        48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
        64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
        80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
        96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
        112, 113, 114, 115, 116, 117, 118, 119, 110, 121, 122, 123, 124, 125, 126, 127
    };

    if (count != 0) {
        // Set Texture IDs & MVP uniforms
        shader.setUniform1iv("u_Textures", count, texture_IDs.data());
        // Draw all required instances
        glDrawElementsInstancedBaseInstance(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, count, offset);
        offset += count;
        count = 0;
    }
}

void PlaneRenderer::_subjectUpdate(Subject const& subject, int event_id)
{
    switch (event_id) {
    case SSS::EventList::Model:
        _model_vbo.needs_edit = true;
        break;
    case SSS::EventList::Alpha:
        _alpha_vbo.needs_edit = true;
        break;
    case SSS::EventList::TexOffset:
        _tex_offset_vbo.needs_edit = true;
        break;
    }
}

template <typename T>
void PlaneRenderer::_updateVBO(T(PlaneBase::* getMember)() const, Basic::VBO& vbo) {
    std::vector<T> vec;
    vec.reserve(_planes.size());
    for (std::shared_ptr<PlaneBase> const& plane : _planes) {
        vec.push_back(((*plane).*getMember)());
    }
    vbo.edit(vec, GL_DYNAMIC_DRAW);
    vbo.needs_edit = false;
};

void PlaneRenderer::render() try
{
    if (!isActive()) {
        return;
    }

    Shaders::Shared shader = getShaders();
    if (!shader)
        return;
    shader->use();
    _vao.bind();

    // Check if we need to reset the depth buffer before rendering
    if (clear_depth_buffer) {
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    // Set VP
    shader->setUniformMat4fv("u_VP", 1, GL_FALSE,
        glm::value_ptr(camera ? camera->getVP() : glm::mat4(1)));

    // Edit VBOs if needed
    if (_update_vbos || _model_vbo.needs_edit)
        _updateVBO(&PlaneBase::getModelMat4, _model_vbo);

    if (_update_vbos || _alpha_vbo.needs_edit)
        _updateVBO(&PlaneBase::getAlpha, _alpha_vbo);

    if (_update_vbos || _tex_offset_vbo.needs_edit)
        _updateVBO(&PlaneBase::getTexOffset, _tex_offset_vbo);

    _update_vbos = false;

    uint32_t count = 0, offset = 0;
    // Loop over each plane
    for (std::shared_ptr<PlaneBase> const& plane : _planes) {
        // Check if we can't cache more instances and need to make a draw call.
        if (count == Window::maxGLSLTextureUnits()) {
            _renderPart(*shader, count, offset);
        }
        if (!plane || !plane->_texture)
            continue;

        // Bind another active texture (set uniform IDs later)
        glActiveTexture(GL_TEXTURE0 + count);
        plane->_texture->bind();

        ++count;
    }
    _renderPart(*shader, count, offset);
    _vao.unbind();
}
CATCH_AND_RETHROW_METHOD_EXC;

void PlaneRenderer::addPlane(std::shared_ptr<PlaneBase> plane)
{
    if (plane) {
        _planes.push_back(plane);
        _observe(*plane);
        _update_vbos = true;
    }
}

void PlaneRenderer::removePlane(std::shared_ptr<PlaneBase> plane)
{
    if (plane) {
        _planes.erase(std::remove_if(
            _planes.begin(),
            _planes.end(),
            [plane](auto&& p1) {
                return p1.get() == plane.get();
            }
        ));
        _ignore(*plane);
        _update_vbos = true;
    }
}

bool PlaneRenderer::_findNearestModel(double x, double y)
{
    _hovered.reset();
    _hovered_z = DBL_MAX;
    if (!isActive()) {
        return false;
    }
    // Loop over each Renderer::Chunk in reverse order
    bool result = false;
    // Retrieve VP for next loop
    glm::mat4 VP(1);
    if (camera) {
        VP = camera->getVP();
    }
    // Loop over each plane in chunk
    for (std::shared_ptr<PlaneBase> const& plane : _planes | std::views::reverse) {
        if (!plane)
            continue;
        // Check if plane is hovered and retrieve its relative depth
        double z = DBL_MAX;
        if (plane->_isHovered(VP, x, y, z)) {
            result = true;
            // allowed diff
            static constexpr double epsilon = 0.0001;
            // Update hovered stats if plane is nearer
            if (z < _hovered_z && (_hovered_z - z) > epsilon) {
                _hovered_z = z;
                _hovered = plane;
            }
        }
    }
    return result;
}

SSS_GL_END;