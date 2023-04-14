#include "GL/Lua.hpp"
#include "GL/Window.hpp"

static std::ostream& operator<<(std::ostream& out, glm::vec3 const& vec)
{
    out << "vec(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
    return out;
}

SSS_GL_BEGIN;

void lua_setup_GL(sol::state& lua)
{
    auto gl = lua["GL"].get_or_create<sol::table>();

    auto shaders = gl.new_usertype<Shaders>("Shaders", sol::no_constructor,
        sol::base_classes, sol::bases<::SSS::Base, Basic::Base>());
    shaders["loadFromFiles"] = &Shaders::loadFromFiles;
    shaders["loadFromStrings"] = &Shaders::loadFromStrings;
    shaders["create"] = sol::overload(
        sol::resolve<Shaders::Shared(GLFWwindow*)>(Shaders::create),
        sol::resolve<Shaders::Shared()>(Shaders::create),
        sol::resolve<Shaders::Shared(std::string const&, std::string const&)>(Shaders::create)
    );

    auto renderer = gl.new_usertype<RendererBase>("RendererBase", sol::no_constructor);
    renderer["shaders"] = sol::property(&RendererBase::getShaders, &RendererBase::setShaders);
    renderer["active"] = sol::property(&RendererBase::isActive, &RendererBase::setActivity);

    auto plane_renderer = gl.new_usertype<PlaneRenderer>("PlaneRenderer", sol::no_constructor,
        sol::base_classes, sol::bases<RendererBase, ::SSS::Base, Basic::Base>());
    plane_renderer["clear_depth_buffer"] = &PlaneRenderer::clear_depth_buffer;
    plane_renderer["camera"] = &PlaneRenderer::camera;
    plane_renderer["planes"] = &PlaneRenderer::planes;
    plane_renderer["create"] = sol::overload(
        sol::resolve<PlaneRenderer::Shared(GLFWwindow*)>(PlaneRenderer::create),
        sol::resolve<PlaneRenderer::Shared()>(PlaneRenderer::create),
        [](Camera* cam) { return PlaneRenderer::create(Camera::get(cam)); },
        [](Camera* cam, bool clear) { return PlaneRenderer::create(Camera::get(cam), clear); }
    );

    auto line_renderer = gl.new_usertype<LineRenderer>("LineRenderer", sol::no_constructor,
        sol::base_classes, sol::bases<RendererBase, ::SSS::Base, Basic::Base>());
    line_renderer["camera"] = &LineRenderer::camera;
    line_renderer["create"] = sol::overload(
        sol::resolve<LineRenderer::Shared(GLFWwindow*)>(LineRenderer::create),
        sol::resolve<LineRenderer::Shared()>(LineRenderer::create)
    );

    auto texture = gl.new_usertype<Texture>("Texture", sol::no_constructor,
        sol::base_classes, sol::bases<::SSS::Base, Basic::Base>());
    texture["type"] = sol::property(&Texture::getType, &Texture::setType);
    texture["loadImage"] = &Texture::loadImage;
    texture["edit"] = &Texture::editRawPixels;
    texture["text_area"] = sol::property(&Texture::getTextArea, &Texture::setTextArea);
    texture["getDimensions"] = sol::resolve<std::tuple<int, int>() const>(&Texture::getCurrentDimensions);
    texture["create"] = sol::overload(
        sol::resolve<Texture::Shared(GLFWwindow*)>(Texture::create),
        sol::resolve<Texture::Shared()>(Texture::create),
        sol::resolve<Texture::Shared(std::string const&)>(Texture::create),
        sol::resolve<Texture::Shared(TR::Area const&)>(Texture::create)
    );
    gl.new_enum<Texture::Type>("TextureType", {
        { "Raw", Texture::Type::Raw },
        { "Text", Texture::Type::Text }
    });

    // Camera (glm required)
    auto camera = gl.new_usertype<Camera>("Camera", sol::no_constructor,
        sol::base_classes, sol::bases<::SSS::Base, Basic::Base>());
    camera["position"] = sol::property(&Camera::getPosition, &Camera::setPosition);
    camera["move"] = &Camera::move;
    camera["rotation"] = sol::property(&Camera::getPosition, &Camera::setPosition);
    camera["rotate"] = &Camera::rotate;
    camera["proj_type"] = sol::property(&Camera::getProjectionType, &Camera::setProjectionType);
    camera["fov"] = sol::property(&Camera::getFOV, &Camera::setFOV);
    camera["z_near"] = sol::property(&Camera::getZNear, &Camera::setZNear);
    camera["z_far"] = sol::property(&Camera::getZFar, &Camera::setZFar);
    camera["create"] = sol::overload(
        sol::resolve<Camera::Shared(GLFWwindow*)>(Camera::create),
        sol::resolve<Camera::Shared()>(Camera::create)
    );
    gl.new_enum<Camera::Projection>("Projection", {
        { "Ortho", Camera::Projection::Ortho },
        { "OrthoFixed", Camera::Projection::OrthoFixed },
        { "Perspective", Camera::Projection::Perspective }
    });

    auto model = gl.new_usertype<ModelBase>("ModelBase", sol::no_constructor);
    model["scaling"] = sol::property(&ModelBase::getScaling, &ModelBase::setScaling);
    model["rotation"] = sol::property(&ModelBase::getRotation, &ModelBase::setRotation);
    model["translation"] = sol::property(&ModelBase::getTranslation, &ModelBase::setTranslation);
    model["scale"] = sol::resolve<void(float)>(&ModelBase::scale);
    model["rotate"] = &ModelBase::rotate;
    model["translate"] = &ModelBase::translate;
    model["isHovered"] = sol::resolve<bool() const>(&ModelBase::isHovered);
    model["isClicked"] = sol::resolve<bool() const>(&ModelBase::isClicked);
    model["isHeld"] = sol::resolve<bool() const>(&ModelBase::isHeld);

    auto plane = gl.new_usertype<Plane>("Plane", sol::no_constructor,
        sol::base_classes, sol::bases<ModelBase, ::SSS::Base, Basic::Base>());
    plane["texture"] = sol::property(&Plane::getTexture, &Plane::setTexture);
    plane["play"] = &Plane::play;
    plane["pause"] = &Plane::pause;
    plane["stop"] = &Plane::stop;
    plane["isPlaying"] = &Plane::isPlaying;
    plane["isPaused"] = &Plane::isPaused;
    plane["isStopped"] = &Plane::isStopped;
    plane["loop"] = sol::property(&Plane::isLooping, &Plane::setLooping);
    plane["alpha"] = sol::property(&Plane::getAlpha, &Plane::setAlpha);
    plane["hitbox"] = sol::property(&Plane::getHitbox, &Plane::setHitbox);
    plane["create"] = sol::overload(
        sol::resolve<Plane::Shared(GLFWwindow*)>(Plane::create),
        sol::resolve<Plane::Shared()>(Plane::create),
        [](Texture* texture) { return Plane::create(Texture::get(texture)); }
    );
    plane["duplicate"] = &Plane::duplicate;
    gl.new_enum<Plane::Hitbox>("PlaneHitbox", {
        { "None", Plane::Hitbox::None },
        { "Alpha", Plane::Hitbox::Alpha },
        { "Full", Plane::Hitbox::Full }
    });

    auto window = gl.new_usertype<Window>("Window", sol::no_constructor,
        sol::base_classes, sol::bases<::SSS::Base>());
    window["blockInputs"] = &Window::blockInputs;
    window["unblockInputs"] = &Window::unblockInputs;
    window["input_stack_time"] = sol::property(&Window::getInputStackTime, &Window::setInputStackTime);

    window["keyIsHeld"] = sol::overload(
        sol::resolve<bool(int) const>(&Window::keyIsHeld),
        sol::resolve<bool(int, int) const>(&Window::keyIsHeld)
    );
    window["keyIsPressed"] = sol::overload(
        sol::resolve<bool(int) const>(&Window::keyIsPressed),
        sol::resolve<bool(int, int) const > (&Window::keyIsPressed)
    );
    window["keyIsReleased"] = &Window::keyIsReleased;
    window["keyCount"] = &Window::keyCount;

    window["clickIsHeld"] = sol::overload(
        sol::resolve<bool(int) const>(&Window::clickIsHeld),
        sol::resolve<bool(int, int) const>(&Window::clickIsHeld)
    );
    window["clickIsPressed"] = sol::overload(
        sol::resolve<bool(int) const>(&Window::clickIsPressed),
        sol::resolve<bool(int, int) const > (&Window::clickIsPressed)
    );
    window["clickIsReleased"] = &Window::clickIsReleased;
    window["clickCount"] = &Window::clickCount;

    window["addRenderer"] = sol::overload(
        sol::resolve<void(RendererBase::Shared, size_t)>(&Window::addRenderer),
        sol::resolve<void(RendererBase::Shared)>(&Window::addRenderer)
    );
    window["removeRenderer"] = &Window::removeRenderer;

    window["fps_limit"] = sol::property(&Window::getFPSLimit, &Window::setFPSLimit);
    window["vsync"] = sol::property(&Window::getVSYNC, &Window::setVSYNC);
    window["title"] = sol::property(&Window::getTitle, &Window::setTitle);
    window["setDimensions"] = &Window::setDimensions;
    window["getDimensions"] = sol::resolve<std::tuple<int, int>() const>(&Window::getDimensions);
    window["w"] = sol::property(&Window::getWidth, &Window::setWidth);
    window["h"] = sol::property(&Window::getHeight, &Window::setHeight);
    window["setPosition"] = &Window::setPosition;
    window["getPosition"] = sol::resolve<std::tuple<int, int>() const>(&Window::getPosition);
    window["x"] = sol::property(&Window::getPosX, &Window::setPosX);
    window["y"] = sol::property(&Window::getPosY, &Window::setPosY);
    window["getCursorPos"] = sol::resolve<std::tuple<int, int>() const>(&Window::getCursorPos);
    window["getCursorDiff"] = sol::resolve<std::tuple<int, int>() const>(&Window::getCursorDiff);
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
    // GLFW_MOUSE_BUTTON_XXX macros
    {
        gl["LEFT_CLICK"]    = GLFW_MOUSE_BUTTON_LEFT;
        gl["MIDDLE_CLICK"]  = GLFW_MOUSE_BUTTON_MIDDLE;
        gl["RIGHT_CLICK"]   = GLFW_MOUSE_BUTTON_RIGHT;
        gl["CLICK_1"]       = GLFW_MOUSE_BUTTON_1;
        gl["CLICK_2"]       = GLFW_MOUSE_BUTTON_2;
        gl["CLICK_3"]       = GLFW_MOUSE_BUTTON_3;
        gl["CLICK_4"]       = GLFW_MOUSE_BUTTON_4;
        gl["CLICK_5"]       = GLFW_MOUSE_BUTTON_5;
        gl["CLICK_6"]       = GLFW_MOUSE_BUTTON_6;
        gl["CLICK_7"]       = GLFW_MOUSE_BUTTON_7;
        gl["CLICK_8"]       = GLFW_MOUSE_BUTTON_8;

    }
}

SSS_GL_END;