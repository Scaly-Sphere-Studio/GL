#define SSS_LUA
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
    // Setup Lua
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::string);
    lua_setup(lua);
    TR::lua_setup_TR(lua);
    GL::lua_setup_GL(lua);

    // Create Window & set context
    lua.unsafe_script_file("Init.lua");
    GL::Window::Shared window = lua["window"];
    GL::Context const context(window);

    // Finish setting up window
    glClearColor(0.3f, 0.3f, 0.3f, 0.f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    window->setCallback(glfwSetKeyCallback, key_callback);
    window->setCallback(glfwSetWindowSizeCallback, resize_callback);

    // Lines
    auto const& line_shader     = GL::Shaders::create();
    line_shader->loadFromFiles("glsl/line.vert", "glsl/line.frag");
    auto const& line_renderer   = GL::Renderer::create<GL::LineRenderer>();
    line_renderer->setShadersID(line_shader->getID());
    line_renderer->castAs<GL::LineRenderer>().camera = lua["camera"];
    using Line = GL::Polyline;
    Line::Shared line[4];
    line[0] = Line::Segment(glm::vec3(-200,  200, 0), glm::vec3( 200,  200, 0), 10.f, glm::vec4(0, 0, 1, 1), Line::JointType::BEVEL, Line::TermType::SQUARE);
    line[1] = Line::Segment(glm::vec3( 200,  200, 0), glm::vec3( 200, -200, 0), 10.f, glm::vec4(0, 1, 0, 1), Line::JointType::BEVEL, Line::TermType::SQUARE);
    line[2] = Line::Segment(glm::vec3( 200, -200, 0), glm::vec3(-200, -200, 0), 10.f, glm::vec4(1, 0, 0, 1), Line::JointType::BEVEL, Line::TermType::SQUARE);
    line[3] = Line::Segment(glm::vec3(-200, -200, 0), glm::vec3(-200,  200, 0), 10.f, glm::vec4(1, 1, 1, 1), Line::JointType::BEVEL, Line::TermType::SQUARE);

    // Plane
    GL::Plane::passive_funcs    = { { 1, passive_plane_func1 } };
    GL::Plane::on_click_funcs   = { { 1, on_click_plane_func1 } };

    // Main loop
    while (!window->shouldClose()) {
        // Poll events, threads, etc
        GL::pollEverything();
        // Script
        lua.unsafe_script_file("Loop.lua");
        // Draw renderers
        window->drawObjects();
        // Swap buffers
        window->printFrame();
    }
}
CATCH_AND_LOG_FUNC_EXC;