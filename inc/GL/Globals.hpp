#ifndef SSS_GL_GLOBALS_HPP
#define SSS_GL_GLOBALS_HPP

#include "Objects/Texture.hpp"
#include "Objects/Models/PlaneRenderer.hpp"
#include "Objects/Models/LineRenderer.hpp"

SSS_GL_BEGIN;

struct CreateArgs final {
    /** Width of the window (auto-shrinks to fit monitor).*/
    int w{ 720 };
    /** Height of the window (auto-shrinks to fit monitor).*/
    int h{ 720 };
    /** Title of the window.*/
    std::string title{ "Untitled" };
    /** Monitor ID of the window (auto-shrinks to closest valid value).*/
    int monitor_id{ 0 };
    /** Whether to put the window in fullscreen.*/
    bool fullscreen{ false };
    /** Whether to maximize the window.*/
    bool maximized{ false };
    /** Whether to iconify the window.*/
    bool iconified{ false };
    /** Whether to hide the window.*/
    bool hidden{ false };
};


SSS_GL_API void createWindow(CreateArgs const& args = CreateArgs());
SSS_GL_API bool windowShouldClose() noexcept;
SSS_GL_API void closeWindow();

SSS_GL_API Shaders::Shared getPresetShaders(uint32_t id) noexcept;

SSS_GL_API void setRenderers(RendererBase::Vector const& renderers) noexcept;
SSS_GL_API RendererBase::Vector const& getRenderers() noexcept;

SSS_GL_API void addRenderer(RendererBase::Shared renderer, size_t index = 0);
SSS_GL_API void removeRenderer(RendererBase::Shared renderer);

SSS_GL_API void drawRenderers();
SSS_GL_API void printBuffer();
SSS_GL_API void saveScreenshot();

using KeyInputs = std::array<Input, GLFW_KEY_LAST + 1>;
using MouseInputs = std::array<Input, GLFW_MOUSE_BUTTON_LAST + 1>;

SSS_GL_API void blockInputs(int unblocking_key = 0) noexcept;
SSS_GL_API void unblockInputs() noexcept;
SSS_GL_API bool areInputsBlocked() noexcept;

void setInputStackTime(std::chrono::milliseconds ms);
std::chrono::milliseconds getInputStackTime() noexcept;

SSS_GL_API KeyInputs const& getKeyInputs() noexcept;

inline bool keyIsHeld(int key) noexcept { return getKeyInputs()[key].is_held(); };
inline bool keyIsHeld(int key, int min_count) noexcept { return getKeyInputs()[key].is_held(min_count); };

inline bool keyIsPressed(int key) noexcept { return getKeyInputs()[key].is_pressed(); };
inline bool keyIsPressed(int key, int min_count) noexcept { return getKeyInputs()[key].is_pressed(min_count); };

inline bool keyIsReleased(int key) noexcept { return getKeyInputs()[key].is_released(); };

inline int keyCount(int key) noexcept { return getKeyInputs()[key].count(); }

SSS_GL_API MouseInputs const& getClickInputs() noexcept;

inline bool clickIsHeld(int button) noexcept { return getClickInputs()[button].is_held(); };
inline bool clickIsHeld(int button, int min_count) noexcept { return getClickInputs()[button].is_held(min_count); };

inline bool clickIsPressed(int button) noexcept { return getClickInputs()[button].is_pressed(); };
inline bool clickIsPressed(int button, int min_count) noexcept { return getClickInputs()[button].is_pressed(min_count); };

inline bool clickIsReleased(int button) noexcept { return getClickInputs()[button].is_released(); };

inline int clickCount(int button) noexcept { return getClickInputs()[button].count(); }


SSS_GL_API void getCursorPos(int& x, int& y) noexcept;
SSS_GL_API void getCursorDiff(int& x, int& y) noexcept;

SSS_GL_API ModelBase::Shared getHoveredModel() noexcept;
SSS_GL_API ModelBase::Shared getClickedModel() noexcept;
SSS_GL_API ModelBase::Shared getHeldModel() noexcept;

template<class Derived>
std::shared_ptr<Derived> getHovered() noexcept {
    ModelBase::Shared model = getHoveredModel();
    if (model == nullptr)
        return nullptr;
    return std::dynamic_pointer_cast<Derived>(model);
}

template<class Derived>
std::shared_ptr<Derived> getClicked() noexcept {
    ModelBase::Shared model = getClickedModel();
    if (model == nullptr)
        return nullptr;
    return std::dynamic_pointer_cast<Derived>(model);
}

template<class Derived>
std::shared_ptr<Derived> getHeld() noexcept {
    ModelBase::Shared model = getHeldModel();
    if (model == nullptr)
        return nullptr;
    return std::dynamic_pointer_cast<Derived>(model);
}

SSS_GL_API long long getFPS() noexcept;
SSS_GL_API void setFPSLimit(int fps_limit) noexcept;
SSS_GL_API int getFPSLimit() noexcept;

SSS_GL_API void setVSYNC(bool state);
SSS_GL_API bool getVSYNC() noexcept;

SSS_GL_API void setTitle(std::string const& title);
SSS_GL_API std::string getTitle() noexcept;

SSS_GL_API void setSize(int w, int h);
SSS_GL_API void getSize(int& w, int& h) noexcept;
SSS_GL_API float getSizeRatio() noexcept;

SSS_GL_API void setPosition(int x0, int y0);
SSS_GL_API void getPosition(int& x0, int& y0) noexcept;

SSS_GL_API void setFullscreen(bool fullscreen, int monitor_id = -1);
SSS_GL_API bool isFullscreen() noexcept;

SSS_GL_API void setIconification(bool iconify);
SSS_GL_API bool isIconified() noexcept;

SSS_GL_API void setMaximization(bool maximize);
SSS_GL_API bool isMaximized() noexcept;

SSS_GL_API void setVisibility(bool show);
SSS_GL_API bool isVisible() noexcept;

SSS_GL_API GLFWwindow* getGLFWwindow() noexcept;
SSS_GL_API uint32_t maxGLSLTextureUnits() noexcept;

INTERNAL_BEGIN;

struct UserCallbacks {
    // Internal callbacks that are called before calling within themselves user callbacks
    GLFWwindowiconifyfun iconify{ nullptr };          // Window iconify
    GLFWwindowsizefun    resize{ nullptr };           // Window resize
    GLFWwindowposfun     position{ nullptr };         // Window position
    GLFWkeyfun           key{ nullptr };              // Window keyboard key
    GLFWcharfun          character{ nullptr };        // Window character input
    GLFWcursorposfun     mouse_position{ nullptr };   // Window mouse position
    GLFWmousebuttonfun   mouse_button{ nullptr };     // Window mouse button
};

extern UserCallbacks user_callbacks;

INTERNAL_END;

template<typename Callback>
void setCallback(Callback(*set)(GLFWwindow*, Callback), Callback callback)
{
    void const* ptr(set);
    if (ptr == (void*)(&glfwSetWindowIconifyCallback)) {
        _internal::user_callbacks.iconify = GLFWwindowiconifyfun(callback);
    }
    else if (ptr == (void*)(&glfwSetWindowSizeCallback)) {
        _internal::user_callbacks.resize = GLFWwindowsizefun(callback);
    }
    else if (ptr == (void*)(&glfwSetWindowPosCallback)) {
        _internal::user_callbacks.position = GLFWwindowposfun(callback);
    }
    else if (ptr == (void*)(&glfwSetKeyCallback)) {
        _internal::user_callbacks.key = GLFWkeyfun(callback);
    }
    else if (ptr == (void*)(&glfwSetCharCallback)) {
        _internal::user_callbacks.character = GLFWcharfun(callback);
    }
    else if (ptr == (void*)(&glfwSetCursorPosCallback)) {
        _internal::user_callbacks.mouse_position = GLFWcursorposfun(callback);
    }
    else if (ptr == (void*)(&glfwSetMouseButtonCallback)) {
        _internal::user_callbacks.mouse_button = GLFWmousebuttonfun(callback);
    }
    else {
        set(getGLFWwindow(), callback);
    }
};

SSS_GL_END;

#endif // SSS_GL_WINDOW_HPP