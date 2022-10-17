#include "SSS/GL/Objects/Models/PlaneRenderer.hpp"
#include "SSS/GL/Objects/Models/Plane.hpp"
#include "SSS/GL/Objects/Texture.hpp"
#include <ranges>

SSS_GL_BEGIN;

PlaneRenderer::PlaneRenderer(std::weak_ptr<Window> window, uint32_t id) try
    : Renderer(window, id), _vao(_window), _vbo(_window), _ibo(_window)
{
    Context const context(_window);

    setShadersID(static_cast<uint32_t>(Shaders::Preset::Plane));

    _vao.bind();
    _vbo.bind();
    _ibo.bind();

    // Edit VBO
    constexpr float vertices[] = {
        // positions          // texture coords (1 - y)
        -0.5f,  0.5f, 0.0f,   0.f, 1.f - 1.f,   // top left
        -0.5f, -0.5f, 0.0f,   0.f, 1.f - 0.f,   // bottom left
         0.5f, -0.5f, 0.0f,   1.f, 1.f - 0.f,   // bottom right
         0.5f,  0.5f, 0.0f,   1.f, 1.f - 1.f    // top right
    };
    _vbo.edit(sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Edit IBO
    constexpr unsigned int indices[] = {
        0, 1, 2,  // first triangle
        2, 3, 0   // second triangle
    };
    _ibo.edit(sizeof(indices), indices, GL_STATIC_DRAW);
}
CATCH_AND_RETHROW_METHOD_EXC;

void PlaneRenderer::_renderPart(Shaders& shader, uint32_t& count, bool reset_depth) const
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
        shader.setUniform1iv("u_Textures", count, &texture_IDs[0]);
        shader.setUniformMat4fv("u_VPs", count, GL_FALSE, &_VPs[0][0][0]);
        shader.setUniformMat4fv("u_Models", count, GL_FALSE, &_Models[0][0][0]);
        shader.setUniform1fv("u_Alphas", count, &_Alphas[0]);
        // Draw all required instances
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, count);
        count = 0;
    }
    // Clear depth buffer if asked to
    if (reset_depth) {
        glClear(GL_DEPTH_BUFFER_BIT);
    }
}

void PlaneRenderer::render() try
{
    if (!isActive()) {
        return;
    }
    Window::Shared const window = _window.lock();
    if (!window) {
        return;
    }
    Context const context(_window);

    Shaders* shader = window->getShaders(getShadersID());
    if (!shader)
        return;
    shader->use();
    _vao.bind();

    uint32_t count = 0;
    _VPs.resize(window->maxGLSLTextureUnits());
    _Models.resize(window->maxGLSLTextureUnits());
    _Alphas.resize(window->maxGLSLTextureUnits());
    // Loop over each Renderer::Chunk
    for (Chunk const& chunk : chunks) {
        // Check if we can't cache more instances and need to make a draw call.
        // Also check if we need to reset the depth buffer before the next loop,
        // in which case the cached instances need to be drawn.
        if (count == window->maxGLSLTextureUnits() || chunk.reset_depth_before) {
            _renderPart(*shader, count, chunk.reset_depth_before);
        }

        // Set VP for next loop
        glm::mat4 VP(1);
        if (chunk.camera) {
            VP = chunk.camera->getVP();
        }

        // Loop over each plane in chunk
        for (Plane::Shared const& plane : chunk.planes) {
            // Check if we can't cache more instances and need to make a draw call.
            if (count == window->maxGLSLTextureUnits()) {
                _renderPart(*shader, count, false);
            }
            if (!plane || !plane->_use_texture)
                continue;
            Texture* texture = window->getTexture(plane->_texture_id);
            if (!texture)
                continue;

            // Store MVP components (set uniforms later)
            _VPs[count] = VP;
            _Models[count] = plane->getModelMat4();
            _Alphas[count] = plane->getAlpha();
            // Bind another active texture (set uniform IDs later)
            glActiveTexture(GL_TEXTURE0 + count);
            texture->bind();

            ++count;
        }
    }
    _renderPart(*shader, count, false);
    _vao.unbind();
}
CATCH_AND_RETHROW_METHOD_EXC;

bool PlaneRenderer::_findNearestModel(float x, float y)
{
    if (!_hovered.expired())
        _hovered.lock()->_is_hovered = false;
    _hovered.reset();
    _hovered_z = DBL_MAX;
    if (!isActive()) {
        return false;
    }
    // Retrieve window and its objects
    Window::Shared const window = _window.lock();
    if (!window) {
        return false;
    }
    // Loop over each Renderer::Chunk in reverse order
    bool result = false;
    for (Chunk const& chunk : chunks | std::views::reverse) {
        // Retrieve VP for next loop
        glm::mat4 VP(1);
        if (chunk.camera) {
            VP = chunk.camera->getVP();
        }
        // Loop over each plane in chunk
        for (Plane::Shared const& plane : chunk.planes) {
            if (!plane)
                continue;
            // Check if plane is hovered and retrieve its relative depth
            double z = DBL_MAX;
            if (plane->_isHovered(VP, x, y, z)) {
                result = true;
                // Update hovered stats if plane is nearer
                if (z < _hovered_z) {
                    _hovered_z = z;
                    _hovered = plane;
                }
            }
        }
        if (result && chunk.reset_depth_before) {
            break;
        }
    }
    if (!_hovered.expired())
        _hovered.lock()->_is_hovered = true;
    return result;
}

SSS_GL_END;