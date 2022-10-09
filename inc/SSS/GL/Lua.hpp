#pragma once

#ifdef SSS_LUA

#include <sol/sol.hpp>
#include "SSS/GL/Objects/Texture.hpp"
#include "SSS/GL/Objects/Models/PlaneRenderer.hpp"
#include "SSS/GL/Objects/Models/LineRenderer.hpp"

SSS_GL_BEGIN;

void lua_setup_GL(sol::state& lua)
{
    auto gl = lua["GL"].get_or_create<sol::table>();

    auto shaders = gl.new_usertype<Shaders>("Shaders", sol::no_constructor);
    shaders["loadFromFiles"] = &Shaders::loadFromFiles;
    shaders["loadFromStrings"] = &Shaders::loadFromStrings;

    auto renderer = gl.new_usertype<Renderer>("Renderer");
    renderer["shader_id"] = sol::property(&Renderer::getShadersID, &Renderer::setShadersID);
    renderer["active"] = sol::property(&Renderer::isActive, &Renderer::setActivity);

    auto texture = gl.new_usertype<Texture>("Texture", sol::no_constructor);
    texture["type"] = sol::property(&Texture::getType, &Texture::setType);
    texture["loadImage"] = &Texture::loadImage;
    texture["text_area_id"] = sol::property(&Texture::getTextAreaID, &Texture::setTextAreaID);
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
    camera["create"] = &Camera::create;
    gl.new_enum<Camera::Projection>("Projection", {
        { "Ortho", Camera::Projection::Ortho },
        { "OrthoFixed", Camera::Projection::OrthoFixed },
        { "Perspective", Camera::Projection::Perspective }
    });

    auto plane = gl.new_usertype<Plane>("Plane", sol::no_constructor);
    plane["scaling"] = sol::property(&Plane::getScaling, &Plane::setScaling);
    plane["rotation"] = sol::property(&Plane::getRotation, &Plane::setRotation);
    plane["position"] = sol::property(&Plane::getTranslation, &Plane::setTranslation);
    plane["scale"] = &Plane::scale;
    plane["rotate"] = &Plane::rotate;
    plane["move"] = &Plane::translate;
    plane["texture_id"] = sol::property(&Plane::getTextureID, &Plane::setTextureID);
    plane["create"] = &Plane::create;
    plane["duplicate"] = &Plane::duplicate;

    auto window = gl.new_usertype<Window>("Window", sol::no_constructor);
    window["blockInputs"] = &Window::blockInputs;
    window["unblockInputs"] = &Window::unblockInputs;
    window["inputs"] = sol::property(&Window::getKeyInputs);
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
}

SSS_GL_END;

#endif