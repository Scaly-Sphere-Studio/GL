#define SSS_LUA
#include "GL.hpp"
#include <FastNoise/FastNoise.h>
#include <algorithm>

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
    if (auto area = TR::Area::getFocused(); area)
        area->scroll(y * -40);
}

int main() try
{
    Log::Async::louden(true);

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
    GL::PlaneRenderer& plane_renderer = lua["plane_renderer"];

    Log::GL::Window::get().fps = true;

    // Noise texture demo
    {
        constexpr int noise_w = 256, noise_h = 256;
        auto fnGenerator = FastNoise::New<FastNoise::Perlin>();
        // Perlin defaults to Scale=100 (Frequency=0.01), which stacks with the
        // xStepSize/yStepSize below and made the 0.02f step ~100x weaker than
        // intended (a near-flat, uniform-gray result). Neutralize it here.
        fnGenerator->SetScale(1.0f);
        std::vector<float> noise(noise_w * noise_h);
        fnGenerator->GenUniformGrid2D(noise.data(), 0, 0, noise_w, noise_h, 0.02f, 0.02f, 1337);

        RGBA32::Vector pixels(noise.size());
        for (size_t i = 0; i < noise.size(); ++i) {
            uint8_t v = static_cast<uint8_t>(std::clamp((noise[i] * 0.5f + 0.5f) * 255.f, 0.f, 255.f));
            pixels[i] = RGBA32(v, v, v, 255);
        }

        auto noise_texture = GL::Texture::create();
        noise_texture->editRawPixels(pixels.data(), noise_w, noise_h);

        auto noise_plane = GL::Plane::create(noise_texture);
        noise_plane->scale(300.f);
        noise_plane->translate(glm::vec3(0.f, -350.f, 1.f));
        plane_renderer.addPlane(noise_plane);
    }

    // GL::Window& win2 = GL::Window::create();
    // win2.setRenderers(window.getRenderers());

    // Finish setting up window
    glClearColor(0.3f, 0.3f, 0.3f, 0.f);
    window.setCallback(glfwSetKeyCallback, key_callback);
    window.setCallback(glfwSetScrollCallback, scroll_callback);
    // win2.setCallback(glfwSetWindowCloseCallback, close_callback);

    // Lines
    using Line = GL::Polyline;
    Line::Shared line;
    //Line::Shared line[4];
    //line[0] = Line::Segment(glm::vec3(-200,  200, 0), glm::vec3( 200,  200, 0), 10.f, glm::vec4(0, 0, 1, 1), Line::JointType::BEVEL, Line::TermType::SQUARE);
    //line[1] = Line::Segment(glm::vec3( 200,  200, 0), glm::vec3( 200, -200, 0), 10.f, glm::vec4(0, 1, 0, 1), Line::JointType::BEVEL, Line::TermType::SQUARE);
    //line[2] = Line::Segment(glm::vec3( 200, -200, 0), glm::vec3(-200, -200, 0), 10.f, glm::vec4(1, 0, 0, 1), Line::JointType::BEVEL, Line::TermType::SQUARE);
    //line[3] = Line::Segment(glm::vec3(-200, -200, 0), glm::vec3(-200,  200, 0), 10.f, glm::vec4(1, 1, 1, 1), Line::JointType::BEVEL, Line::TermType::SQUARE);

    // Main loop
    while (!window.shouldClose()) {
        // Poll events, threads, etc
        GL::pollEverything();
        auto [w, h] = window.getDimensions();
        auto [x, y] = window.getCursorPos();
        glm::vec3 a, b, c, d;
        a = glm::vec3(0, 0, 0);
        d = glm::vec3(w / -2 + x, h / 2 - y, 0);
        glm::vec3 offset(std::abs(a.x - d.x) / 2.f, 0, 0);
        b = a - offset;
        c = d + offset;
        if (window.keyIsPressed(GLFW_KEY_SPACE)) {
            LOG_MSG("SPACE")
        }
        line = Line::Bezier(a, b, c, d, 20.f, glm::vec4(1, 1, 1, 1), Line::JointType::BEVEL, Line::TermType::SQUARE);
        // Script
        lua.safe_script_file("Loop.lua");
        // Draw renderers
        window.drawObjects();
        // Swap buffers
        window.printFrame();

        if (window.getClickInputs()[GLFW_MOUSE_BUTTON_1].is_pressed(2))
            window.placeHoveredTextAreaCursor();

        //if (window.keyIsPressed(GLFW_KEY_F1))
            // win2.setVisibility(true);

        // win2.drawObjects();
        // win2.printFrame();
    }

    window.close();
}
CATCH_AND_LOG_FUNC_EXC;