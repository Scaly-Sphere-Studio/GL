#define SSS_LUA
#include "GL.hpp"

using namespace SSS;

void key_callback(GLFWwindow* ptr, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
        case GLFW_KEY_KP_0:
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(ptr, true);
            break;
        }
    }
}

void close_callback(GLFWwindow* win)
{
    GL::Window* window = GL::Window::get(win);
    if (!window)
        return;

    window->setVisibility(false);
    glfwSetWindowShouldClose(win, false);
}

void scroll_callback(GLFWwindow* win, double x, double y)
{
    TR::Area::get(0)->scroll(y * -40);
}

int main() try
{
    Log::GL::Callbacks::get().window_resize = true;
    // Setup Lua
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::string);
    lua_setup(lua);
    TR::lua_setup_TR(lua);
    GL::lua_setup_GL(lua);

    // Create Window & set context
    lua.safe_script_file("Init.lua");
    GL::Window& window = lua["window"];

    Log::GL::Window::get().fps = true;

    // GL::Window& win2 = GL::Window::create();
    // win2.setRenderers(window.getRenderers());

    // Finish setting up window
    glClearColor(0.3f, 0.3f, 0.3f, 0.f);
    window.setCallback(glfwSetKeyCallback, key_callback);
    window.setCallback(glfwSetScrollCallback, scroll_callback);
    // win2.setCallback(glfwSetWindowCloseCallback, close_callback);

    // Lines
    using Line = GL::Polyline;
    Line::Shared line[4];
    line[0] = Line::Segment(glm::vec3(-200,  200, 0), glm::vec3( 200,  200, 0), 10.f, glm::vec4(0, 0, 1, 1), Line::JointType::BEVEL, Line::TermType::SQUARE);
    line[1] = Line::Segment(glm::vec3( 200,  200, 0), glm::vec3( 200, -200, 0), 10.f, glm::vec4(0, 1, 0, 1), Line::JointType::BEVEL, Line::TermType::SQUARE);
    line[2] = Line::Segment(glm::vec3( 200, -200, 0), glm::vec3(-200, -200, 0), 10.f, glm::vec4(1, 0, 0, 1), Line::JointType::BEVEL, Line::TermType::SQUARE);
    line[3] = Line::Segment(glm::vec3(-200, -200, 0), glm::vec3(-200,  200, 0), 10.f, glm::vec4(1, 1, 1, 1), Line::JointType::BEVEL, Line::TermType::SQUARE);

    // Main loop
    while (!window.shouldClose()) {
        // Poll events, threads, etc
        GL::pollEverything();
        // Script
        lua.safe_script_file("Loop.lua");
        // Draw renderers
        window.drawObjects();
        // Swap buffers
        window.printFrame();

        //if (window.keyIsPressed(GLFW_KEY_F1))
            // win2.setVisibility(true);

        // win2.drawObjects();
        // win2.printFrame();
    }

    window.close();
}
CATCH_AND_LOG_FUNC_EXC;