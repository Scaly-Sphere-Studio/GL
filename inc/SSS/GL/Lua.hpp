#ifndef SSS_GL_LUA_HPP
#define SSS_GL_LUA_HPP

#ifdef SSS_LUA

#include <sol/sol.hpp>
#include "SSS/GL/Objects/Texture.hpp"
#include "SSS/GL/Objects/Models/PlaneRenderer.hpp"
#include "SSS/GL/Objects/Models/LineRenderer.hpp"

inline std::ostream& operator<<(std::ostream& out, glm::vec3 const& vec)
{
    out << "vec(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
    return out;
}

SSS_GL_BEGIN;

inline void lua_setup_GL(sol::state& lua)
{
    auto gl = lua["GL"].get_or_create<sol::table>();

    auto shaders = gl.new_usertype<Shaders>("Shaders", sol::no_constructor);
    shaders["loadFromFiles"] = &Shaders::loadFromFiles;
    shaders["loadFromStrings"] = &Shaders::loadFromStrings;

    auto renderer = gl.new_usertype<Renderer>("Renderer", sol::no_constructor);
    renderer["shader_id"] = sol::property(&Renderer::getShadersID, &Renderer::setShadersID);
    renderer["active"] = sol::property(&Renderer::isActive, &Renderer::setActivity);
    renderer["title"] = &Renderer::title;

    auto plane_renderer = gl.new_usertype<PlaneRenderer>("PlaneRenderer",
        sol::no_constructor, sol::base_classes, sol::bases<Renderer>());
    plane_renderer["chunks"] = &PlaneRenderer::chunks;
    plane_renderer["create"] = []() {
        return std::ref(Renderer::create<PlaneRenderer>()->castAs<PlaneRenderer>()); };
    auto chunk = gl.new_usertype<PlaneRenderer::Chunk>("Chunk", sol::constructors<
        PlaneRenderer::Chunk(),
        PlaneRenderer::Chunk(Camera::Shared),
        PlaneRenderer::Chunk(Camera::Shared, bool)
    >());
    chunk["title"] = &PlaneRenderer::Chunk::title;
    chunk["reset_depth_before"] = &PlaneRenderer::Chunk::reset_depth_before;
    chunk["camera"] = &PlaneRenderer::Chunk::camera;
    chunk["planes"] = &PlaneRenderer::Chunk::planes;

    auto texture = gl.new_usertype<Texture>("Texture", sol::no_constructor);
    texture["type"] = sol::property(&Texture::getType, &Texture::setType);
    texture["loadImage"] = &Texture::loadImage;
    texture["edit"] = &Texture::editRawPixels;
    texture["text_area_id"] = sol::property(&Texture::getTextAreaID, &Texture::setTextAreaID);
    texture["create"] = [](Window::Shared win) { return Texture::create(win).get(); };
    texture["id"] = sol::property(&Texture::getID);
    gl.new_enum<Texture::Type>("TextureType", {
        { "Raw", Texture::Type::Raw },
        { "Text", Texture::Type::Text }
    });

    // Camera (glm required)
    auto camera = gl.new_usertype<Camera>("Camera", sol::no_constructor);
    camera["position"] = sol::property(&Camera::getPosition, &Camera::setPosition);
    camera["move"] = &Camera::move;
    camera["rotation"] = sol::property(&Camera::getPosition, &Camera::setPosition);
    camera["rotate"] = &Camera::rotate;
    camera["proj_type"] = sol::property(&Camera::getProjectionType, &Camera::setProjectionType);
    camera["fov"] = sol::property(&Camera::getFOV, &Camera::setFOV);
    camera["z_near"] = sol::property(&Camera::getZNear, &Camera::setZNear);
    camera["z_far"] = sol::property(&Camera::getZFar, &Camera::setZFar);
    camera["create"] = sol::overload(
        sol::resolve<Camera::Shared(Window::Shared)>(Camera::create), 
        sol::resolve<Camera::Shared()>(Camera::create)),
    gl.new_enum<Camera::Projection>("Projection", {
        { "Ortho", Camera::Projection::Ortho },
        { "OrthoFixed", Camera::Projection::OrthoFixed },
        { "Perspective", Camera::Projection::Perspective }
    });

    auto plane = gl.new_usertype<Plane>("Plane", sol::no_constructor);
    plane["scaling"] = sol::property(&Plane::getScaling, &Plane::setScaling);
    plane["rotation"] = sol::property(&Plane::getRotation, &Plane::setRotation);
    plane["translation"] = sol::property(&Plane::getTranslation, &Plane::setTranslation);
    plane["scale"] = &Plane::scale;
    plane["rotate"] = &Plane::rotate;
    plane["translate"] = &Plane::translate;
    plane["texture_id"] = sol::property(&Plane::getTextureID, &Plane::setTextureID);
    plane["passive_func_id"] = sol::property(&Plane::getPassiveFuncID, &Plane::setPassiveFuncID);
    plane["on_click_func_id"] = sol::property(&Plane::getOnClickFuncID, &Plane::setOnClickFuncID);
    plane["hitbox"] = sol::property(&Plane::getHitbox, &Plane::setHitbox);
    plane["create"] = sol::overload(
        sol::resolve<Plane::Shared(Window::Shared)>(Plane::create),
        sol::resolve<Plane::Shared()>(Plane::create)),
    plane["duplicate"] = &Plane::duplicate;
    gl.new_enum<Plane::Hitbox>("PlaneHitbox", {
        { "None", Plane::Hitbox::None },
        { "Alpha", Plane::Hitbox::Alpha },
        { "Full", Plane::Hitbox::Full }
    });

    auto window = gl.new_usertype<Window>("Window", sol::no_constructor);
    window["blockInputs"] = &Window::blockInputs;
    window["unblockInputs"] = &Window::unblockInputs;
    window["keyIsPressed"] = &Window::keyIsPressed;
    //window["objects"] = sol::property(&Window::getObjects);

    window["fps_limit"] = sol::property(&Window::getFPSLimit, &Window::setFPSLimit);
    window["vsync"] = sol::property(&Window::getVSYNC, &Window::setVSYNC);
    window["title"] = sol::property(&Window::getTitle, &Window::setTitle);
    window["getDimensions"] = [](Window& win) {
        int w, h; win.getDimensions(w, h); return std::make_tuple(w, h); };
    window["setDimensions"] = &Window::setDimensions;
    window["w"] = sol::property(&Window::getWidth, &Window::setWidth);
    window["h"] = sol::property(&Window::getHeight, &Window::setHeight);
    window["getPosition"] = [](Window& win) {
        int x, y; win.getPosition(x, y); return std::make_tuple(x, y); };
    window["x"] = sol::property(&Window::getPosX, &Window::setPosX);
    window["y"] = sol::property(&Window::getPosY, &Window::setPosY);
    window["setPosition"] = &Window::setPosition;
    window["fullscreen"] = sol::property(&Window::isFullscreen, &Window::setFullscreen);
    window["iconified"] = sol::property(&Window::isIconified, &Window::setIconification);
    window["maximized"] = sol::property(&Window::isMaximized, &Window::setMaximization);
    window["visible"] = sol::property(&Window::isVisible, &Window::setVisibility);
    window["create"] = &Window::create;
    // Window args
    auto args = gl.new_usertype<Window::CreateArgs>("WindowArgs");
    args["w"] = &Window::CreateArgs::w;
    args["h"] = &Window::CreateArgs::h;
    args["title"] = &Window::CreateArgs::title;
    args["monitor_id"] = &Window::CreateArgs::monitor_id;
    args["fullscreen"] = &Window::CreateArgs::fullscreen;
    args["maximized"] = &Window::CreateArgs::maximized;
    args["iconified"] = &Window::CreateArgs::iconified;
    args["hidden"] = &Window::CreateArgs::hidden;
    
    // Context
    auto context = gl.new_usertype<Context>("Context",
        sol::constructors<Context(Window::Shared)>());

    auto vec3 = lua.new_usertype<glm::vec3>("vec3", sol::constructors<
        glm::vec3(),
        glm::vec3(float),
        glm::vec3(float, float, float)
    >());
    vec3["x"] = &glm::vec3::x;
    vec3["y"] = &glm::vec3::y;
    vec3["z"] = &glm::vec3::z;

    // GLFW_KEY_XXX macros
    {
        gl["KEY_SPACE"]         = GLFW_KEY_SPACE;
        gl["KEY_APOSTROPHE"]    = GLFW_KEY_APOSTROPHE;
        gl["KEY_COMMA"]         = GLFW_KEY_COMMA;
        gl["KEY_MINUS"]         = GLFW_KEY_MINUS;
        gl["KEY_PERIOD"]        = GLFW_KEY_PERIOD;
        gl["KEY_SLASH"]         = GLFW_KEY_SLASH;
        gl["KEY_0"]             = GLFW_KEY_0;
        gl["KEY_1"]             = GLFW_KEY_1;
        gl["KEY_2"]             = GLFW_KEY_2;
        gl["KEY_3"]             = GLFW_KEY_3;
        gl["KEY_4"]             = GLFW_KEY_4;
        gl["KEY_5"]             = GLFW_KEY_5;
        gl["KEY_6"]             = GLFW_KEY_6;
        gl["KEY_7"]             = GLFW_KEY_7;
        gl["KEY_8"]             = GLFW_KEY_8;
        gl["KEY_9"]             = GLFW_KEY_9;
        gl["KEY_SEMICOLON"]     = GLFW_KEY_SEMICOLON;
        gl["KEY_EQUAL"]         = GLFW_KEY_EQUAL;
        gl["KEY_A"]             = GLFW_KEY_A;
        gl["KEY_B"]             = GLFW_KEY_B;
        gl["KEY_C"]             = GLFW_KEY_C;
        gl["KEY_D"]             = GLFW_KEY_D;
        gl["KEY_E"]             = GLFW_KEY_E;
        gl["KEY_F"]             = GLFW_KEY_F;
        gl["KEY_G"]             = GLFW_KEY_G;
        gl["KEY_H"]             = GLFW_KEY_H;
        gl["KEY_I"]             = GLFW_KEY_I;
        gl["KEY_J"]             = GLFW_KEY_J;
        gl["KEY_K"]             = GLFW_KEY_K;
        gl["KEY_L"]             = GLFW_KEY_L;
        gl["KEY_M"]             = GLFW_KEY_M;
        gl["KEY_N"]             = GLFW_KEY_N;
        gl["KEY_O"]             = GLFW_KEY_O;
        gl["KEY_P"]             = GLFW_KEY_P;
        gl["KEY_Q"]             = GLFW_KEY_Q;
        gl["KEY_R"]             = GLFW_KEY_R;
        gl["KEY_S"]             = GLFW_KEY_S;
        gl["KEY_T"]             = GLFW_KEY_T;
        gl["KEY_U"]             = GLFW_KEY_U;
        gl["KEY_V"]             = GLFW_KEY_V;
        gl["KEY_W"]             = GLFW_KEY_W;
        gl["KEY_X"]             = GLFW_KEY_X;
        gl["KEY_Y"]             = GLFW_KEY_Y;
        gl["KEY_Z"]             = GLFW_KEY_Z;
        gl["KEY_LEFT_BRACKET"]  = GLFW_KEY_LEFT_BRACKET;
        gl["KEY_BACKSLASH"]     = GLFW_KEY_BACKSLASH;
        gl["KEY_RIGHT_BRACKET"] = GLFW_KEY_RIGHT_BRACKET;
        gl["KEY_GRAVE_ACCENT"]  = GLFW_KEY_GRAVE_ACCENT;
        gl["KEY_WORLD_1"]       = GLFW_KEY_WORLD_1;
        gl["KEY_WORLD_2"]       = GLFW_KEY_WORLD_2;
        gl["KEY_ESCAPE"]        = GLFW_KEY_ESCAPE;
        gl["KEY_ENTER"]         = GLFW_KEY_ENTER;
        gl["KEY_TAB"]           = GLFW_KEY_TAB;
        gl["KEY_BACKSPACE"]     = GLFW_KEY_BACKSPACE;
        gl["KEY_INSERT"]        = GLFW_KEY_INSERT;
        gl["KEY_DELETE"]        = GLFW_KEY_DELETE;
        gl["KEY_RIGHT"]         = GLFW_KEY_RIGHT;
        gl["KEY_LEFT"]          = GLFW_KEY_LEFT;
        gl["KEY_DOWN"]          = GLFW_KEY_DOWN;
        gl["KEY_UP"]            = GLFW_KEY_UP;
        gl["KEY_PAGE_UP"]       = GLFW_KEY_PAGE_UP;
        gl["KEY_PAGE_DOWN"]     = GLFW_KEY_PAGE_DOWN;
        gl["KEY_HOME"]          = GLFW_KEY_HOME;
        gl["KEY_END"]           = GLFW_KEY_END;
        gl["KEY_CAPS_LOCK"]     = GLFW_KEY_CAPS_LOCK;
        gl["KEY_SCROLL_LOCK"]   = GLFW_KEY_SCROLL_LOCK;
        gl["KEY_NUM_LOCK"]      = GLFW_KEY_NUM_LOCK;
        gl["KEY_PRINT_SCREEN"]  = GLFW_KEY_PRINT_SCREEN;
        gl["KEY_PAUSE"]         = GLFW_KEY_PAUSE;
        gl["KEY_F1"]            = GLFW_KEY_F1;
        gl["KEY_F2"]            = GLFW_KEY_F2;
        gl["KEY_F3"]            = GLFW_KEY_F3;
        gl["KEY_F4"]            = GLFW_KEY_F4;
        gl["KEY_F5"]            = GLFW_KEY_F5;
        gl["KEY_F6"]            = GLFW_KEY_F6;
        gl["KEY_F7"]            = GLFW_KEY_F7;
        gl["KEY_F8"]            = GLFW_KEY_F8;
        gl["KEY_F9"]            = GLFW_KEY_F9;
        gl["KEY_F10"]           = GLFW_KEY_F10;
        gl["KEY_F11"]           = GLFW_KEY_F11;
        gl["KEY_F12"]           = GLFW_KEY_F12;
        gl["KEY_F13"]           = GLFW_KEY_F13;
        gl["KEY_F14"]           = GLFW_KEY_F14;
        gl["KEY_F15"]           = GLFW_KEY_F15;
        gl["KEY_F16"]           = GLFW_KEY_F16;
        gl["KEY_F17"]           = GLFW_KEY_F17;
        gl["KEY_F18"]           = GLFW_KEY_F18;
        gl["KEY_F19"]           = GLFW_KEY_F19;
        gl["KEY_F20"]           = GLFW_KEY_F20;
        gl["KEY_F21"]           = GLFW_KEY_F21;
        gl["KEY_F22"]           = GLFW_KEY_F22;
        gl["KEY_F23"]           = GLFW_KEY_F23;
        gl["KEY_F24"]           = GLFW_KEY_F24;
        gl["KEY_F25"]           = GLFW_KEY_F25;
        gl["KEY_KP_0"]          = GLFW_KEY_KP_0;
        gl["KEY_KP_1"]          = GLFW_KEY_KP_1;
        gl["KEY_KP_2"]          = GLFW_KEY_KP_2;
        gl["KEY_KP_3"]          = GLFW_KEY_KP_3;
        gl["KEY_KP_4"]          = GLFW_KEY_KP_4;
        gl["KEY_KP_5"]          = GLFW_KEY_KP_5;
        gl["KEY_KP_6"]          = GLFW_KEY_KP_6;
        gl["KEY_KP_7"]          = GLFW_KEY_KP_7;
        gl["KEY_KP_8"]          = GLFW_KEY_KP_8;
        gl["KEY_KP_9"]          = GLFW_KEY_KP_9;
        gl["KEY_KP_DECIMAL"]    = GLFW_KEY_KP_DECIMAL;
        gl["KEY_KP_DIVIDE"]     = GLFW_KEY_KP_DIVIDE;
        gl["KEY_KP_MULTIPLY"]   = GLFW_KEY_KP_MULTIPLY;
        gl["KEY_KP_SUBTRACT"]   = GLFW_KEY_KP_SUBTRACT;
        gl["KEY_KP_ADD"]        = GLFW_KEY_KP_ADD;
        gl["KEY_KP_ENTER"]      = GLFW_KEY_KP_ENTER;
        gl["KEY_KP_EQUAL"]      = GLFW_KEY_KP_EQUAL;
        gl["KEY_LEFT_SHIFT"]    = GLFW_KEY_LEFT_SHIFT;
        gl["KEY_LEFT_CONTROL"]  = GLFW_KEY_LEFT_CONTROL;
        gl["KEY_LEFT_ALT"]      = GLFW_KEY_LEFT_ALT;
        gl["KEY_LEFT_SUPER"]    = GLFW_KEY_LEFT_SUPER;
        gl["KEY_RIGHT_SHIFT"]   = GLFW_KEY_RIGHT_SHIFT;
        gl["KEY_RIGHT_CONTROL"] = GLFW_KEY_RIGHT_CONTROL;
        gl["KEY_RIGHT_ALT"]     = GLFW_KEY_RIGHT_ALT;
        gl["KEY_RIGHT_SUPER"]   = GLFW_KEY_RIGHT_SUPER;
        gl["KEY_MENU"]          = GLFW_KEY_MENU;
    }
}

SSS_GL_END;

#endif

#endif // SSS_GL_LUA_HPP