#include "GL/Objects/Models/PlaneRenderer.hpp"
#include "GL/Window.hpp"
#include <ranges>

SSS_GL_BEGIN;

PlaneRendererBase::PlaneRendererBase() try
{
    setShaders(Window::getPresetShaders(static_cast<uint32_t>(Shaders::Preset::Plane)));

    // Edit VBO
    constexpr float vertices[] = {
        // positions          // texture coords (1 - y)
        -0.5f,  0.5f, 0.0f,   0.f, 1.f - 1.f,   // top left
        -0.5f, -0.5f, 0.0f,   0.f, 1.f - 0.f,   // bottom left
         0.5f, -0.5f, 0.0f,   1.f, 1.f - 0.f,   // bottom right
         0.5f,  0.5f, 0.0f,   1.f, 1.f - 1.f    // top right
    };
    _vbo.edit(sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Edit IBO
    constexpr unsigned int indices[] = {
        0, 1, 2,  // first triangle
        2, 3, 0   // second triangle
    };
    _ibo.edit(sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Setup VAO
    _vao.setup([this]() {
        _vbo.bind();
        _ibo.bind();
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    });
    _vao.unbind();
}
CATCH_AND_RETHROW_METHOD_EXC;

void PlaneRendererBase::_renderPart(Shaders& shader, uint32_t& count) const
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
        shader.setUniform1fv("u_TextureOffsets", count, &_TextureOffsets[0]);
        shader.setUniform1fv("u_Alphas", count, &_Alphas[0]);
        // Draw all required instances
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, count);
        count = 0;
    }
}

void PlaneRendererBase::render() try
{
    if (!isActive()) {
        return;
    }

    Shaders::Shared shader = getShaders();
    if (!shader)
        return;
    shader->use();
    _vao.bind();

    uint32_t count = 0;
    _VPs.resize(Window::maxGLSLTextureUnits());
    _Models.resize(Window::maxGLSLTextureUnits());
    _TextureOffsets.resize(Window::maxGLSLTextureUnits());
    _Alphas.resize(Window::maxGLSLTextureUnits());
    // Check if we need to reset the depth buffer before rendering
    if (clear_depth_buffer) {
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    // Set VP
    glm::mat4 VP(1);
    if (camera) {
        VP = camera->getVP();
    }

    // Loop over each plane
    for (Plane::Shared const& plane : planes) {
        // Check if we can't cache more instances and need to make a draw call.
        if (count == Window::maxGLSLTextureUnits()) {
            _renderPart(*shader, count);
        }
        if (!plane || !plane->_texture)
            continue;

        // Store MVP components (set uniforms later)
        _VPs[count] = VP;
        _Models[count] = plane->getModelMat4();
        _TextureOffsets[count] = static_cast<float>(plane->_texture_offset);
        _Alphas[count] = plane->getAlpha();
        // Bind another active texture (set uniform IDs later)
        glActiveTexture(GL_TEXTURE0 + count);
        plane->_texture->bind();

        ++count;
    }
    _renderPart(*shader, count);
    _vao.unbind();
}
CATCH_AND_RETHROW_METHOD_EXC;

bool PlaneRendererBase::_findNearestModel(double x, double y)
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
    for (Plane::Shared const& plane : planes | std::views::reverse) {
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