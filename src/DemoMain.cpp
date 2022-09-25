#include "SSS/GL.hpp"

using namespace SSS;

void key_callback(GLFWwindow* ptr, int key, int scancode, int action, int mods)
{
    GL::Window::Shared const window = GL::Window::get(ptr);

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
        case GLFW_KEY_KP_0:
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(ptr, true);
            break;
        }
    }
}

void resize_callback(GLFWwindow* win, int w, int h)
{
}

void passive_plane_func1(GL::Window::Shared win, GL::Plane::Shared plane)
{
    //if (plane->isHovered())
    //    plane->rotate(glm::vec3(0, 0, 1));
}

void on_click_plane_func1(GL::Window::Shared win, GL::Plane::Shared plane,
    int button, int action, int mod)
{
    //if (button == GLFW_MOUSE_BUTTON_1 &&  action == GLFW_PRESS)
    //    plane->rotate(glm::vec3(0, 0, 45));
}

int main() try
{
    // Create Window
    GL::Window::CreateArgs args;
    args.title = "SSS/GL - Demo Window";
    args.w = static_cast<int>(600);
    args.h = static_cast<int>(600);
    GL::Window::Shared window = GL::Window::create(args);

    // Set context
    GL::Context const context(window);

    // Finish setting up window
    glClearColor(0.3f, 0.3f, 0.3f, 0.f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    window->setVSYNC(true);
    window->setCallback(glfwSetKeyCallback, key_callback);
    window->setCallback(glfwSetWindowSizeCallback, resize_callback);

    // SSS/GL objects

    // Create objects
    auto const& texture         = GL::Texture::create();
    auto const camera           = GL::Camera::create();
    auto const& line_shader     = GL::Shaders::create();
    auto const& line_renderer   = GL::Renderer::create<GL::LineRenderer>();
    auto const plane            = GL::Plane::create();
    auto const& plane_renderer  = GL::Renderer::create<GL::PlaneRenderer>();

    // Text
    auto const& area = TR::Area::create(300, 300);
    auto fmt = area->getFormat();
    fmt.style.charsize = 50;
    fmt.color.text.func = TR::Format::Color::Func::rainbow;
    area->setFormat(fmt);
    area->parseString("Lorem ipsum dolor sit amet.");
    texture->setTextAreaID(area->getID());
    texture->setType(GL::Texture::Type::Text);

    // Camera
    camera->setPosition({ 0, 0, 3 });
    camera->setProjectionType(GL::Camera::Projection::OrthoFixed);

    // Lines
    line_shader->loadFromFiles("glsl/line.vert", "glsl/line.frag");
    line_renderer->setShadersID(line_shader->getID());
    line_renderer->castAs<GL::LineRenderer>().camera = camera;
    using Line = GL::Polyline;
    Line::Shared line[4];
    line[0] = Line::Segment(glm::vec3(-200,  200, 0), glm::vec3( 200,  200, 0), 10.f, glm::vec4(0, 0, 1, 1), Line::JointType::BEVEL, Line::TermType::SQUARE);
    line[1] = Line::Segment(glm::vec3( 200,  200, 0), glm::vec3( 200, -200, 0), 10.f, glm::vec4(0, 1, 0, 1), Line::JointType::BEVEL, Line::TermType::SQUARE);
    line[2] = Line::Segment(glm::vec3( 200, -200, 0), glm::vec3(-200, -200, 0), 10.f, glm::vec4(1, 0, 0, 1), Line::JointType::BEVEL, Line::TermType::SQUARE);
    line[3] = Line::Segment(glm::vec3(-200, -200, 0), glm::vec3(-200,  200, 0), 10.f, glm::vec4(1, 1, 1, 1), Line::JointType::BEVEL, Line::TermType::SQUARE);

    // Plane
    plane->setTextureID(texture->getID());
    plane->scale(glm::vec3(300));
    GL::Plane::passive_funcs    = { { 1, passive_plane_func1 } };
    GL::Plane::on_click_funcs   = { { 1, on_click_plane_func1 } };
    plane->setPassiveFuncID(1);
    plane->setOnClickFuncID(1);
    plane->setHitbox(GL::Plane::Hitbox::Full);
    {
        auto& chunks = plane_renderer->castAs<GL::PlaneRenderer>().chunks;
        // Display 256 textures
        chunks.emplace_back(camera);
        constexpr int size = 10;
        RGBA32::Vector pixels(size * size);
        for (int i = 0; i < 256; ++i) {
            // Create Plane & Texture
            auto const plane    = GL::Plane::create();
            auto const& texture = GL::Texture::create();
            // Fill texture
            std::fill(pixels.begin(), pixels.end(), (0xFF << 24) + (i << 16) + (i << 8) + i);
            texture->editRawPixels(&pixels[0], size, size);
            // Edit plane
            plane->setTextureID(texture->getID());
            plane->scale(glm::vec3(20));
            plane->translate(glm::vec3(-150 + 20 * (i / 16), -150 + (i % 16) * 20, 0));
            // Display plane
            chunks.back().planes.emplace_back(plane);
        }
        // Display previous text last
        chunks.emplace_back(camera, true);
        chunks.back().planes.emplace_back(plane);
    }

    // Main loop
    while (!window->shouldClose()) {
        // Poll events, threads, etc
        GL::pollEverything();
        // Handle key inputs
        auto const& keys = window->getKeyInputs();
        float constexpr speed = 1.5f;
        if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])
            camera->move(glm::vec3(0,  speed, 0));
        if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])
            camera->move(glm::vec3(0, -speed, 0));
        if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])
            camera->move(glm::vec3(-speed, 0, 0));
        if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])
            camera->move(glm::vec3( speed, 0, 0));
        // Draw renderers
        window->drawObjects();
        // Swap buffers
        window->printFrame();
    }
}
CATCH_AND_LOG_FUNC_EXC;