#include "SSS/GL/Plane.hpp"
#include "SSS/GL/Window.hpp"

__SSS_GL_BEGIN

PlaneRenderer::PlaneRenderer(std::weak_ptr<Window> window) try
    : Renderer(window)
{
    Context const context(_window);

    std::string const vertex = "\
#version 330 core\n\
layout(location = 0) in vec3 a_Pos;\n\
layout(location = 1) in vec2 a_UV;\n\
\n\
uniform mat4 u_Models[" + std::to_string(glsl_max_array_size) + "];\n\
uniform mat4 u_VPs[" + std::to_string(glsl_max_array_size) + "];\n\
\n\
out vec2 UV;\n\
flat out int instanceID;\n\
\n\
void main()\n\
{\n\
    gl_Position = u_VPs[gl_InstanceID] * u_Models[gl_InstanceID] * vec4(a_Pos, 1);\n\
    UV = a_UV;\n\
    instanceID = gl_InstanceID;\n\
}";
    std::string const fragment = "\
#version 330 core\n\
out vec4 FragColor;\n\
\n\
in vec2 UV;\n\
flat in int instanceID;\n\
\n\
uniform sampler2D u_Textures[" + std::to_string(glsl_max_array_size) + "];\n\
\n\
void main()\n\
{\n\
    FragColor = texture(u_Textures[instanceID], UV);\n\
}";

    _shaders->loadFromData(vertex, fragment);

    _vao->bind();
    _vbo->bind();
    _ibo->bind();

    constexpr float vertices[] = {
        // positions          // texture coords (1 - y)
        -0.5f,  0.5f, 0.0f,   0.0f, 1.f - 1.0f,   // top left
        -0.5f, -0.5f, 0.0f,   0.0f, 1.f - 0.0f,   // bottom left
         0.5f, -0.5f, 0.0f,   1.0f, 1.f - 0.0f,   // bottom right
         0.5f,  0.5f, 0.0f,   1.0f, 1.f - 1.0f    // top right
    };
    _vbo->edit(sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    constexpr unsigned int indices[] = {
        0, 1, 2,  // first triangle
        2, 3, 0   // second triangle
    };
    _ibo->edit(sizeof(indices), indices, GL_STATIC_DRAW);
}
__CATCH_AND_RETHROW_METHOD_EXC

void PlaneRenderer::_renderPart(uint32_t& count, bool reset_depth) const
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
        _shaders->setUniform1iv("u_Textures", count, &texture_IDs[0]);
        _shaders->setUniformMat4fv("u_VPs", count, GL_FALSE, &_VPs[0][0][0]);
        _shaders->setUniformMat4fv("u_Models", count, GL_FALSE, &_Models[0][0][0]);
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
    if (!_is_active) {
        return;
    }
    Window::Shared const window = _window.lock();
    if (!window) {
        return;
    }
    Context const context(_window);
    Window::Objects const& objects = window->getObjects();
    _shaders->use();
    _vao->bind();

    uint32_t count = 0;
    // Loop over each RenderChunk
    for (RenderChunk const& chunk : *this) {
        // Check if we can't cache more instances and need to make a draw call.
        // Also check if we need to reset the depth buffer before the next loop,
        // in which case the cached instances need to be drawn.
        if (count == glsl_max_array_size || chunk.reset_depth_before) {
            _renderPart(count, chunk.reset_depth_before);
        }

        // Set VP for next loop
        glm::mat4 VP(1);
        if (chunk.use_camera) {
            // Retrieve Camera
            if (objects.cameras.count(chunk.camera_ID) == 0)
                continue;
            Camera::Ptr const& camera = objects.cameras.at(chunk.camera_ID);
            if (!camera)
                continue;
            VP = camera->getProjection() * camera->getView();
        }

        // Loop over each plane in chunk
        for (uint32_t const& object_id : chunk.objects) {
            // Check if we can't cache more instances and need to make a draw call.
            if (count == glsl_max_array_size) {
                _renderPart(count, false);
            }
            // Retrieve Plane and Texture
            if (objects.planes.count(object_id) == 0)
                continue;
            Plane::Ptr const& plane = objects.planes.at(object_id);
            if (!plane)
                continue;
            if (!plane->_use_texture || objects.textures.count(plane->_texture_id) == 0)
                continue;
            Texture::Ptr const& texture = objects.textures.at(plane->_texture_id);
            if (!texture)
                continue;

            // Store MVP components (set uniforms later)
            _VPs[count] = VP;
            _Models[count] = plane->getModelMat4();
            // Bind another active texture (set uniform IDs later)
            glActiveTexture(GL_TEXTURE0 + count);
            texture->bind();

            ++count;
        }
    }
    _renderPart(count, false);
}
__CATCH_AND_RETHROW_METHOD_EXC

bool PlaneRenderer::_findNearestModel(float x, float y)
{
    _hovered_plane = 0;
    _hovered_z = DBL_MAX;
    if (!_is_active) {
        return false;
    }
    // Retrieve window and its objects
    Window::Shared const window = _window.lock();
    if (!window) {
        return false;
    }
    Window::Objects const& objects = window->getObjects();
    // Loop over each RenderChunk in reverse order
    bool result = false;
    for (auto it = crbegin(); it != crend(); ++it) {
        RenderChunk const& chunk = *it;
        // Retrieve VP for next loop
        glm::mat4 VP(1);
        if (chunk.use_camera) {
            // Retrieve Camera
            if (objects.cameras.count(chunk.camera_ID) == 0)
                continue;
            Camera::Ptr const& camera = objects.cameras.at(chunk.camera_ID);
            if (!camera)
                continue;
            VP = camera->getProjection() * camera->getView();
        }
        // Loop over each plane in chunk
        for (uint32_t const& object_id : chunk.objects) {
            // Retrieve Plane
            if (objects.planes.count(object_id) == 0)
                continue;
            Plane::Ptr const& plane = objects.planes.at(object_id);
            if (!plane)
                continue;
            // Check if plane is hovered and retrieve its relative depth
            double z = DBL_MAX;
            if (plane->_isHovered(VP, x, y, z)) {
                result = true;
                // Update hovered stats if plane is nearer
                if (z < _hovered_z) {
                    _hovered_z = z;
                    _hovered_plane = object_id;
                }
            }
        }
        if (result && chunk.reset_depth_before) {
            return result;
        }
    }
    return result;
}

Plane::Plane(std::weak_ptr<Window> window) try
    : Model(window)
{
}
__CATCH_AND_RETHROW_METHOD_EXC

Plane::~Plane()
{
}

void Plane::setTextureID(uint32_t texture_id)
{
    _texture_id = texture_id;
    _use_texture = true;
    _updateTexScaling();
}

glm::mat4 Plane::getModelMat4()
{
    if (_should_compute_mat4) {
        glm::mat4 scaling = glm::scale(_scaling, _tex_scaling);
        _model_mat4 = _translation * _rotation * scaling;
        _should_compute_mat4 = false;
    }
    return _model_mat4;
}

void Plane::getAllTransformations(glm::vec3& scaling, glm::vec3& rot_angles, glm::vec3& translation)
{
    Model::getAllTransformations(scaling, rot_angles, translation);
    scaling /= _tex_scaling;
}

void Plane::_updateTexScaling()
{
    // Retrieve texture dimensions
    Window::Shared const window = _window.lock();
    if (!window || !_use_texture) {
        _tex_scaling = glm::vec3(1);
        _should_compute_mat4 = true;
        return;
    }
    // Check if dimensions changed
    int w, h;
    window->getObjects().textures.at(_texture_id)->getDimensions(w, h);
    if (_tex_w == w && _tex_h == h) {
        return;
    }
    // Update dimensions
    _tex_w = w;
    _tex_h = h;

    // Get texture ratio
    float const ratio = (static_cast<float>(_tex_w) / static_cast<float>(_tex_h));
    // Calculate according scaling
    glm::vec3 scaling(1);
    if (ratio < 1.f) {
        scaling[1] = 1 / ratio;
    }
    else {
        scaling[0] = ratio;
    }

    // If scaling changed, indicate that the Model matrice should be computed
    if (_tex_scaling != scaling) {
        _tex_scaling = scaling;
        _should_compute_mat4 = true;
    }
}

bool Plane::_hoverTriangle(glm::mat4 const& mvp, glm::vec4 const& A,
    glm::vec4 const& B, glm::vec4 const& C, float x, float y,
    double& z, bool& is_hovered)
{
    // Skip if one (or more) of the points is behind the camera
    if (A.z > 1.f || B.z > 1.f || C.z > 1.f) {
        return false;
    }

    // Check if P is inside the triangle ABC via barycentric coordinates
    float const denominator = ((B.y - C.y) * (A.x - C.x) + (C.x - B.x) * (A.y - C.y));
    float const a = ((B.y - C.y) * (x - C.x) + (C.x - B.x) * (y - C.y)) / denominator;
    float const b = ((C.y - A.y) * (x - C.x) + (A.x - C.x) * (y - C.y)) / denominator;
    float const c = 1.f - a - b;
    if (!(0.f <= a && a <= 1.f && 0.f <= b && b <= 1.f && 0.f <= c && c <= 1.f)) {
        return false;
    }
    // Compute relative Z (not the real scene Z, as it's computed via
    // normalized vertices)
    z = a * A.z + b * B.z + c * C.z;
    // If z < -1.f, then it is behind the camera
    if (z < -1.f) {
        return true;
    }

    // Inverse and normalize relative x/y in a -0.5/+0.5 range
    glm::vec4 P = glm::inverse(mvp) * glm::vec4(x, y, z, 1);
    P /= P.w;
    // Convert x/y in a 0/1 range, and inverse y (openGL y VS texture y)
    P.x += 0.5f;
    P.y += 0.5f;
    P.y = 1.f - P.y;
    // Get the relative x & y position the cursor is hovering over.
    _relative_x = static_cast<int>(P.x * static_cast<float>(_tex_w));
    _relative_y = static_cast<int>(P.y * static_cast<float>(_tex_h));

    // End function if no texture provided, or if the hitbox doesn't care about alpha.
    if (!_use_texture || _hitbox == Hitbox::Full) {
        is_hovered = true;
        return true;
    }

    // Retrieve Texture.
    Window::Shared const window = _window.lock();
    if (!window) {
        return true;
    }
    Window::Objects const& objects = window->getObjects();
    if (objects.textures.count(_texture_id) == 0) {
        return true;
    }
    Texture::Ptr const& texture = objects.textures.at(_texture_id);
    if (!texture) {
        return true;
    }

    // Update status if the position is on an opaque pixel
    size_t const pixel = static_cast<size_t>(_relative_y * _tex_w + _relative_x);
    if (texture->_type == Texture::Type::Raw) {
        if (pixel < texture->_pixels.size()) {
            is_hovered = texture->_pixels.at(pixel).bytes.a != 0;
        }
    }
    else if (texture->_type == Texture::Type::Text) {
        TR::TextArea::Ptr const& text_area = texture->getTextArea();
        if (text_area) {
            int w, h;
            text_area->getDimensions(w, h);
            size_t const size = static_cast<size_t>(w) * static_cast<size_t>(h);
            if (pixel < size) {
                void const* pixels = text_area->getPixels();
                is_hovered = static_cast<RGBA32 const*>(pixels)[pixel].bytes.a != 0;
            }
        }
    }

    return true;
}

// Updates _is_hovered via the mouse position callback.
bool Plane::_isHovered(glm::mat4 const& VP, float x, float y, double &z) try
{
    // Skip if no hitbox
    if (_hitbox == Hitbox::None) {
        return false;
    }

    // Plane MVP matrix
    glm::mat4 const mvp = VP * getModelMat4();
    // Plane's coordinates
    glm::vec4 const A4 = mvp * glm::vec4(-0.5, 0.5, 0, 1);   // Top left
    glm::vec4 const B4 = mvp * glm::vec4(-0.5, -0.5, 0, 1);  // Bottom left
    glm::vec4 const C4 = mvp * glm::vec4(0.5, -0.5, 0, 1);   // Bottom right
    glm::vec4 const D4 = mvp * glm::vec4(0.5, 0.5, 0, 1);    // Top right
    // Normalize in screen coordinates
    glm::vec4 const A = A4 / A4.w;
    glm::vec4 const B = B4 / B4.w;
    glm::vec4 const C = C4 / C4.w;
    glm::vec4 const D = D4 / D4.w;

    // Test for ABC or CDA
    bool is_hovered = false;
    _hoverTriangle(mvp, A, B, C, x, y, z, is_hovered)
        || _hoverTriangle(mvp, C, D, A, x, y, z, is_hovered);
    return is_hovered;
}
__CATCH_AND_RETHROW_METHOD_EXC;

__SSS_GL_END