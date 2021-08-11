#include "SSS/GL/Plane.hpp"
#include "SSS/GL/Window.hpp"

__SSS_GL_BEGIN

Plane::Plane(std::weak_ptr<Window> window) try
    : Model(window)
{
    Context const context(_window);

    constexpr float vertices[] = {
        // positions          // texture coords (1 - y)
        -0.5f,  0.5f, 0.0f,   0.0f, 1.f - 1.0f,   // top left
        -0.5f, -0.5f, 0.0f,   0.0f, 1.f - 0.0f,   // bottom left
         0.5f, -0.5f, 0.0f,   1.0f, 1.f - 0.0f,   // bottom right
         0.5f,  0.5f, 0.0f,   1.0f, 1.f - 1.0f    // top right
    };
    constexpr unsigned int indices[] = {
        0, 1, 2,  // first triangle
        0, 2, 3   // second triangle
    };

    _vao->bind();
    _vbo->bind();
    _ibo->bind();

    _vbo->edit(sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    _ibo->edit(sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}
__CATCH_AND_RETHROW_METHOD_EXC

Plane::~Plane()
{
}

void Plane::useTexture(uint32_t texture_id)
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

void Plane::draw() const try
{
    Window::Shared const window = _window.lock();
    if (!window) {
        return;
    }
    Context const context(_window);
    _vao->bind();
    Shaders::Ptr const& shader = window->getObjects().shaders.at(0);
    // Bind texture if needed
    if (_use_texture) {

        glUniform1i(shader->getUniformLocation("u_Textures[0]"), 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, window->getObjects().textures.at(0)->_raw_texture.id);

        glUniform1i(shader->getUniformLocation("u_Textures[1]"), 1);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, window->getObjects().textures.at(2)->_raw_texture.id);
    }
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, 2);
}
__CATCH_AND_RETHROW_METHOD_EXC

// Sets the function to be called when the button is clicked.
// The function MUST be of the format void (*)();
void Plane::setFunction(ButtonFunction func) try
{
    _f = func;
}
__CATCH_AND_RETHROW_METHOD_EXC

// Calls the function set via setFunction();
// Called whenever the button is clicked.
void Plane::callFunction() try
{
    if (!_use_as_button) {
        return;
    }
    Window::Shared const window = _window.lock();
    if (window && _use_texture) {
        Texture::Ptr const& texture = window->getObjects().textures.at(_texture_id);
        if (texture->getType() == Texture::Type::Text) {
            texture->getTextArea()->placeCursor(_relative_x, _relative_y);
        }
    }
    if (_f != nullptr) {
        _f();
    }
}
__CATCH_AND_RETHROW_METHOD_EXC

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

bool Plane::_hoverTriangle(glm::vec3 const& A, glm::vec3 const& B,
    glm::vec3 const& C, glm::vec3 const& P, bool is_abc)
{
    // Skip if one (or more) of the points is behind the camera
    if (A.z > 1.f || B.z > 1.f || C.z > 1.f) {
        return false;
    }

    // Check if P is inside the triangle ABC via barycentric coordinates
    float const denominator = ((B.y - C.y) * (A.x - C.x) + (C.x - B.x) * (A.y - C.y));
    float const a = ((B.y - C.y) * (P.x - C.x) + (C.x - B.x) * (P.y - C.y)) / denominator;
    float const b = ((C.y - A.y) * (P.x - C.x) + (A.x - C.x) * (P.y - C.y)) / denominator;
    float const c = 1.f - a - b;
    if (!(0.f <= a && a <= 1.f && 0.f <= b && b <= 1.f && 0.f <= c && c <= 1.f)) {
        return false;
    }

    // End function if no texture provided or if the Plane isn't drawn as a rectangle
    if (!_use_texture || !(A.x == B.x && B.y == C.y)) {
        _is_hovered = true;
        return true;
    }

    // Get the relative x & y position the cursor is hovering over.
    // Inverse x & y based on if the triangle is ABC or CDA
    _relative_x = static_cast<int>(c * static_cast<float>(_tex_w));
    _relative_y = static_cast<int>(a * static_cast<float>(_tex_h));
    if (is_abc) {
        _relative_y = _tex_h - _relative_y;
    }
    else {
        _relative_x = _tex_w - _relative_x;
    }

    // Retrieve window, ensure it exists. Then retrieve the texture
    // and ensure it exists too. Skip if the texture is Text.
    Window::Shared const window = _window.lock();
    if (!window) {
        return true;
    }
    Texture::Ptr const& texture = window->getObjects().textures.at(_texture_id);
    if (!texture) {
        return true;
    }
    if (texture->getType() == Texture::Type::Text) {
        _is_hovered = true;
        return true;
    }

    // Update status if the position is inside the texture
    size_t const pixel = static_cast<size_t>(_relative_y * _tex_w + _relative_x);
    if (pixel < texture->_pixels.size()) {
        _is_hovered = texture->_pixels.at(pixel).bytes.a != 0;
    }

    return true;
}

// Updates _is_hovered via the mouse position callback.
void Plane::_updateHoverStatus(double x, double y) try
{
    // Skip if not used as a button
    if (!_use_as_button) {
        return;
    }

    // Reset hover status
    _is_hovered = false;

    // Plane MVP matrix
    glm::mat4 const mvp = getMVP();
    // Plane's coordinates
    glm::vec4 const A4 = mvp * glm::vec4(-0.5, 0.5, 0, 1);   // Top left
    glm::vec4 const B4 = mvp * glm::vec4(-0.5, -0.5, 0, 1);  // Bottom left
    glm::vec4 const C4 = mvp * glm::vec4(0.5, -0.5, 0, 1);   // Bottom right
    glm::vec4 const D4 = mvp * glm::vec4(0.5, 0.5, 0, 1);    // Top right
    // Normalize in screen coordinates
    glm::vec3 const A = A4 / A4.w;
    glm::vec3 const B = B4 / B4.w;
    glm::vec3 const C = C4 / C4.w;
    glm::vec3 const D = D4 / D4.w;
    // Mouse coordinates
    glm::vec3 const P(x, y, 0);

    if (_hoverTriangle(A, B, C, P, true)) {
        return;
    }
    _hoverTriangle(C, D, A, P, false);
}
__CATCH_AND_RETHROW_METHOD_EXC;

PlaneRenderer::PlaneRenderer(std::weak_ptr<Window> window)
    : Renderer(window)
{
    Context const context(_window);

    constexpr char vertex[] =
"#version 330 core\n\
layout(location = 0) in vec3 a_Pos;\n\
layout(location = 1) in vec2 a_UV;\n\
\n\
uniform mat4 u_MVPs[128];\n\
\n\
out vec2 UV;\n\
flat out int instanceID;\n\
\n\
void main()\n\
{\n\
    gl_Position = u_MVPs[gl_InstanceID] * vec4(a_Pos, 1);\n\
    UV = a_UV;\n\
    instanceID = gl_InstanceID;\n\
}";
    constexpr char fragment[] =
"#version 330 core\n\
out vec4 FragColor;\n\
\n\
in vec2 UV;\n\
flat in int instanceID;\n\
\n\
uniform sampler2D u_Textures[128];\n\
\n\
void main()\n\
{\n\
    FragColor = texture(u_Textures[instanceID], UV);\n\
}";

    _shaders->loadFromData(vertex, fragment);

    _vbo.reset(new VBO(window));
    _ibo.reset(new IBO(window));

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
    constexpr unsigned int indices[] = {
        0, 1, 2,  // first triangle
        2, 3, 0   // second triangle
    };

    _vbo->edit(sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    _ibo->edit(sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void PlaneRenderer::render() const
{
    Window::Shared const window = _window.lock();
    if (!window) {
        return;
    }
    Context const context(_window);
    Window::Objects const& objects = window->getObjects();
    
    _shaders->use();
    _vao->bind();

    constexpr uint32_t max_instances = 128;
    std::array<glm::mat4, max_instances> MVPs;
    static constexpr std::array<int32_t, max_instances> texture_IDs = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
        10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
        20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
        30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
        40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
        50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
        60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
        70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
        80, 81, 82, 83, 84, 85, 86, 87, 88, 89,
        90, 91, 92, 93, 94, 95, 96, 97, 98, 99,
        100, 101, 102, 103, 104, 105, 106, 107, 108, 109,
        110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
        110, 121, 122, 123, 124, 125, 126, 127
    };
    uint32_t count = 0;
    for (RenderObject const& render_object : *this) {
        // Ensure we don't try to draw more than we can in one go
        if (count == max_instances) {
            // Bind Texture IDs & MVP
            glUniform1iv(_shaders->getUniformLocation("u_Textures"), count, &texture_IDs[0]);
            _shaders->setUniformMat4("u_MVPs", count, GL_FALSE, &MVPs[0][0][0]);
            glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, count);
            count = 0;
        }
        
        // Retrieve Plane, Camera, and Texture
        if (objects.planes.count(render_object.object_id) == 0)
            continue;
        Plane::Ptr const& plane = objects.planes.at(render_object.object_id);
        if (!plane)
            continue;
        if (objects.cameras.count(render_object.camera_id) == 0)
            continue;
        Camera::Ptr const& camera = objects.cameras.at(render_object.camera_id);
        if (!camera)
            continue;
        if (objects.textures.count(plane->_texture_id) == 0)
            continue;
        Texture::Ptr const& texture = objects.textures.at(plane->_texture_id);
        if (!camera)
            continue;

        // Compute MVP
        MVPs[count] = camera->getMVP(plane->getModelMat4());
        // Bind texture
        glActiveTexture(GL_TEXTURE0 + count);
        texture->bind();

        ++count;
    }
    // Bind Texture IDs & MVP
    glUniform1iv(_shaders->getUniformLocation("u_Textures"), count, &texture_IDs[0]);
    _shaders->setUniformMat4("u_MVPs", count, GL_FALSE, &MVPs[0][0][0]);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, count);
}

__SSS_GL_END