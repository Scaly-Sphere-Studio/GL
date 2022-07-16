#include "SSS/GL.hpp"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_KP_0 || key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, true);
    }

}

void resize_callback(GLFWwindow* win, int w, int h)
{
}

int main() try
{
    using namespace SSS;
    
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
    auto const& texture = GL::Texture::create();
    auto const& camera = GL::Camera::create();
    auto const& line_shader = window->createShaders();
    auto const& line_renderer = window->createRenderer<GL::LineRenderer>();
    auto const& plane = GL::Plane::create();
    auto const& plane_renderer = GL::Plane::Renderer::create();

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
    line_renderer->castAs<GL::LineRenderer>().cam_id = camera->getID();
    using Line = GL::Polyline;
    Line::Shared line[4];
    line[0] = Line::Segment(glm::vec3(-200, 200, 0), glm::vec3(200, 200, 0), 10.f, glm::vec4(0, 0, 1, 1), Line::JointType::BEVEL, Line::TermType::SQUARE);
    line[1] = Line::Segment(glm::vec3(200, 200, 0), glm::vec3(200, -200, 0), 10.f, glm::vec4(0, 1, 0, 1), Line::JointType::BEVEL, Line::TermType::SQUARE);
    line[2] = Line::Segment(glm::vec3(200, -200, 0), glm::vec3(-200, -200, 0), 10.f, glm::vec4(1, 0, 0, 1), Line::JointType::BEVEL, Line::TermType::SQUARE);
    line[3] = Line::Segment(glm::vec3(-200, -200, 0), glm::vec3(-200, 200, 0), 10.f, glm::vec4(1, 1, 1, 1), Line::JointType::BEVEL, Line::TermType::SQUARE);

    // Plane
    plane->setTextureID(texture->getID());
    plane->scale(glm::vec3(300));
    plane_renderer->chunks.emplace_back();
    plane_renderer->chunks[0].reset_depth_before = true;
    plane_renderer->chunks[0].objects.push_back(0);

    // Main loop
    while (!window->shouldClose()) {
        // Poll events, threads, etc
        GL::pollEverything();
        // Draw renderers
        window->drawObjects();
        // Swap buffers
        window->printFrame();
    }
}
CATCH_AND_LOG_FUNC_EXC;