#include <GL/Objects/Models/UIRenderer.hpp>
#include <SceneGraph/scenegraph.h>
#include <SceneGraph/Node_UI.h>
#include <GL/Window.hpp>
#include <GL/Objects/Shaders.hpp>
#include <algorithm>

SSS_GL_BEGIN;

UIRenderer::UIRenderer()
{
    addMaterial("default", Material(SSS::GL::Shaders::create("glsl/ui.vert", "glsl/ui.frag")));

    _proj = glm::ortho(
        0.0f, 1440.f,      // left, right
        810.f, 0.0f,       // bottom, top (Y down)
        -20.f, 20.f        // near, far
    );

    // === Setup SDF shape rendering ===
    std::vector<float> vertices = {
        0.0f, 1.0f, 0.0f,  // top
        0.0f, 0.0f, 0.0f,  // left
        1.0f, 0.0f, 0.0f,  // right
        1.0f, 1.0f, 0.0f,  // top
        1.0f, 0.0f, 0.0f,  // left
        0.0f, 1.0f, 0.0f   // right
    };

    _sdf_vbo.edit(vertices, GL_STATIC_DRAW);
    _sdf_vao.setup([this]() {
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    });

    glGenBuffers(1, &_sdf_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _sdf_ssbo);
    size_t maxElements = 1024;
    glBufferData(GL_SHADER_STORAGE_BUFFER, maxElements * sizeof(UIPrimitive), nullptr, GL_DYNAMIC_DRAW);
    _sdf_vao.unbind();

    // === Setup plane rendering ===
    _setupPlaneVAO();

    // Non-instanced VAO for SDF planes: only a_Pos/a_UV, model & alpha are uniforms
    _sdf_plane_vao.setup([this]() {
        enum {
            SDF_PLANE_POS,
            SDF_PLANE_UV,
        };

        _plane_static_vbo.bind();
        _plane_static_ibo.bind();
        glEnableVertexAttribArray(SDF_PLANE_POS);
        glVertexAttribPointer(SDF_PLANE_POS, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(SDF_PLANE_UV);
        glVertexAttribPointer(SDF_PLANE_UV, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    });
    _sdf_plane_vao.unbind();
}

void UIRenderer::_setupPlaneVAO()
{
    // UV v=0 maps to OpenGL texture bottom = stb_image row 0 = image top.
    // In Y-down ortho, model y=-0.5 is the visual top, so assign v=0 there.
    constexpr float vertices[] = {
        -0.5f,  0.5f, 0.0f,   0.f, 1.f,   // model top-left    → visual bottom → v=1
        -0.5f, -0.5f, 0.0f,   0.f, 0.f,   // model bottom-left → visual top    → v=0
         0.5f, -0.5f, 0.0f,   1.f, 0.f,   // model bottom-right→ visual top    → v=0
         0.5f,  0.5f, 0.0f,   1.f, 1.f,   // model top-right   → visual bottom → v=1
    };
    _plane_static_vbo.edit(sizeof(vertices), vertices, GL_STATIC_DRAW);

    constexpr unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };
    _plane_static_ibo.edit(sizeof(indices), indices, GL_STATIC_DRAW);

    _plane_vao.setup([this]() {
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

        _plane_static_vbo.bind();
        _plane_static_ibo.bind();
        glEnableVertexAttribArray(PLANE_POS);
        glVertexAttribPointer(PLANE_POS, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(PLANE_UV);
        glVertexAttribPointer(PLANE_UV, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

        _plane_model_vbo.bind();
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

        _plane_alpha_vbo.bind();
        glEnableVertexAttribArray(PLANE_ALPHA);
        glVertexAttribPointer(PLANE_ALPHA, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
        glVertexAttribDivisor(PLANE_ALPHA, 1);

        _plane_tex_offset_vbo.bind();
        glEnableVertexAttribArray(PLANE_TEX_OFFSET);
        glVertexAttribIPointer(PLANE_TEX_OFFSET, 1, GL_UNSIGNED_INT, sizeof(uint32_t), (void*)0);
        glVertexAttribDivisor(PLANE_TEX_OFFSET, 1);
    });
    _plane_vao.unbind();
}

void UIRenderer::_updatePlaneVBOs()
{
    std::vector<glm::mat4> models;
    models.reserve(_planes.size());
    for (auto const& plane : _planes) {
        if (!plane || plane->isHidden() || plane->sdf_mode != PlaneBase::SDFMode::None) continue;
        models.push_back(plane->getModelMat4());
    }
    _plane_model_vbo.edit(models, GL_DYNAMIC_DRAW);

    std::vector<float> alphas;
    alphas.reserve(_planes.size());
    for (auto const& plane : _planes) {
        if (!plane || plane->isHidden() || plane->sdf_mode != PlaneBase::SDFMode::None) continue;
        alphas.push_back(plane->getAlpha());
    }
    _plane_alpha_vbo.edit(alphas, GL_DYNAMIC_DRAW);

    std::vector<uint32_t> tex_offsets;
    tex_offsets.reserve(_planes.size());
    for (auto const& plane : _planes) {
        if (!plane || plane->isHidden() || plane->sdf_mode != PlaneBase::SDFMode::None) continue;
        tex_offsets.push_back(plane->getTexOffset());
    }
    _plane_tex_offset_vbo.edit(tex_offsets, GL_DYNAMIC_DRAW);

    _planes_update_vbos = false;
}

void UIRenderer::_renderPlanes()
{
    if (_planes.empty()) return;

    auto plane_shader = SSS::GL::Window::getPresetShaders(static_cast<uint32_t>(Shaders::Preset::Plane));
    if (!plane_shader) return;

    plane_shader->use();
    plane_shader->setUniform("u_VP", _proj);

    _plane_vao.bind();

    if (_planes_update_vbos) {
        _updatePlaneVBOs();
    }

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

    uint32_t count = 0;
    for (auto const& plane : _planes) {
        if (!plane || plane->isHidden() || plane->sdf_mode != PlaneBase::SDFMode::None) continue;
        if (count == 128) break;

        glActiveTexture(GL_TEXTURE0 + count);
        auto texture = plane->getTexture();
        if (texture) {
            texture->bind();
        }
        ++count;
    }

    if (count > 0) {
        plane_shader->setUniform1iv("u_Textures", count, texture_IDs.data());
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, count);
    }

    _plane_vao.unbind();
}

void UIRenderer::_renderPlaneSDF()
{
    auto sdf_shader = SSS::GL::Window::getPresetShaders(static_cast<uint32_t>(Shaders::Preset::PlaneSDF));
    if (!sdf_shader) return;

    // Bind the mask texture on the last hardware-guaranteed unit, away from the low units
    // used by _renderPlanes()'s instanced batch and unit 0 (which uGradientTexture implicitly
    // reads from in ui.frag, relying on whatever texture was last bound there).
    // A hardcoded unit (e.g. 31) can exceed GL_MAX_TEXTURE_IMAGE_UNITS on some hardware/drivers,
    // making glActiveTexture a no-op (GL_INVALID_ENUM) and leaving the sampler reading an
    // incomplete texture (observed as solid white).
    const int maskTexUnit = std::max(1, static_cast<int>(Window::maxGLSLTextureUnits()) - 1);

    sdf_shader->use();
    sdf_shader->setUniform("u_VP", _proj);
    sdf_shader->setUniform("u_Texture", maskTexUnit);

    _sdf_plane_vao.bind();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _sdf_ssbo);

    for (auto const& plane : _planes) {
        if (!plane || plane->isHidden() || plane->sdf_mode == PlaneBase::SDFMode::None)
            continue;
        if (plane->sdf_prims.empty())
            continue;

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _sdf_ssbo);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,
            plane->sdf_prims.size() * sizeof(SSS::UIPrimitive),
            plane->sdf_prims.data());

        sdf_shader->setUniform("u_Model",    plane->getModelMat4());
        sdf_shader->setUniform("u_Alpha",    plane->getAlpha());
        sdf_shader->setUniform("u_SDFMode",  static_cast<int>(plane->sdf_mode));
        sdf_shader->setUniform("u_PrimSize", static_cast<int>(plane->sdf_prims.size()));

        if (plane->sdf_mode == PlaneBase::SDFMode::Mask && plane->getTexture()) {
            glActiveTexture(GL_TEXTURE0 + maskTexUnit);
            plane->getTexture()->bind();
            sdf_shader->setUniform("u_TexOffset", static_cast<int>(plane->getTexOffset()));
        }

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    }
    _sdf_plane_vao.unbind();
}

void UIRenderer::_renderSDFShapes()
{
    auto& mat = _materials.at("default");
    mat.set("uFrameRes", _resolution);
    mat.set("uProgress", 0.f);
    mat.set("uProj", _proj);
    mat.set("uSize", _resolution);
    mat.set("uPos", glm::vec3(0, 0, 0));
    mat.set("uGradientID", 0);
    mat.bind();

    _sdf_vao.bind();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _sdf_ssbo);

    for (const auto& node : SSS::SceneGraph::get().list) {
        Node_UI* n = reinterpret_cast<Node_UI*>(SceneGraph::at(node));
        if (!n || n->isHidden() || n->prims.empty()) continue;
        if (std::find(_worldNodes.begin(), _worldNodes.end(), n) != _worldNodes.end()) continue;

        mat.set("uPrimSize", (int)n->prims.size());

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _sdf_ssbo);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,
            n->prims.size() * sizeof(UIPrimitive),
            n->prims.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    _sdf_vao.unbind();
}

void UIRenderer::pushWorld(Node_UI* n)
{
    if (n && std::find(_worldNodes.begin(), _worldNodes.end(), n) == _worldNodes.end())
        _worldNodes.push_back(n);
}

void UIRenderer::_renderWorldSDFShapes()
{
    if (!_worldCamera || _worldNodes.empty()) return;

    float zoom = _worldCamera->getZoom();
    glm::vec2 vpSize = _resolution / zoom;
    glm::vec2 vpMin  = glm::vec2(_worldCamera->getPosition()) - vpSize * 0.5f;

    auto& mat = _materials.at("default");
    mat.set("uFrameRes", vpSize);
    mat.set("uProgress", 0.f);
    mat.set("uProj",     _worldCamera->getVP());
    mat.set("uSize",     vpSize);
    mat.set("uPos",      glm::vec3(vpMin, 0.f));
    mat.set("uGradientID", 0);
    mat.bind();

    _sdf_vao.bind();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _sdf_ssbo);

    for (Node_UI* n : _worldNodes) {
        if (!n || n->isHidden() || n->prims.empty()) continue;

        mat.set("uPrimSize", (int)n->prims.size());

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _sdf_ssbo);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,
            n->prims.size() * sizeof(UIPrimitive),
            n->prims.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    _sdf_vao.unbind();
}

void UIRenderer::render()
{
    // World-space SDF pass runs first, depth-tested against what box_renderer wrote.
    // The SDF quad sits at z=-1 so it loses where boxes (z=0) already own the depth buffer.
    glDepthFunc(GL_LEQUAL);
    _renderWorldSDFShapes();

    // Clear depth so the screen-space passes always render on top of world geometry.
    glClear(GL_DEPTH_BUFFER_BIT);
    _renderPlanes();
    _renderPlaneSDF();
    _renderSDFShapes();
}

void UIRenderer::updateResolution(const float _w, const float _h)
{
    _resolution = glm::vec2(_w, _h);
    _proj = glm::ortho(
        0.0f, _w,              // left, right
        _h, 0.0f,              // bottom, top (Y down)
        -20.f, 20.f            // near, far
    );
}

void UIRenderer::push(Node_UI* n)
{
    SceneGraph::emplace(n);
    SceneGraph::get().list.push_back(n->_key);
    if (_window) n->observe(_window);
}

void UIRenderer::addPlane(std::shared_ptr<PlaneBase> plane)
{
    if (plane) {
        _planes.push_back(plane);
        _observe(*plane);
        _planes_update_vbos = true;
    }
}

void UIRenderer::removePlane(std::shared_ptr<PlaneBase> plane)
{
    if (plane) {
        _planes.erase(std::remove_if(
            _planes.begin(),
            _planes.end(),
            [plane](auto&& p) { return p.get() == plane.get(); }
        ), _planes.end());
        _ignore(*plane);
        _planes_update_vbos = true;
    }
}

void UIRenderer::_subjectUpdate(Subject const& subject, Event const& event)
{
    int const event_id = event.id;
    if (event_id == EVENT_ID("SSS_MODEL_UPDATE")) {
        _planes_update_vbos = true;
        return;
    }
    if (event_id == EVENT_ID("SSS_PLANE_ALPHA")) {
        _planes_update_vbos = true;
        return;
    }
    if (event_id == EVENT_ID("SSS_PLANE_TEXTURE_OFFSET")) {
        _planes_update_vbos = true;
        return;
    }
}

SSS_GL_END;
