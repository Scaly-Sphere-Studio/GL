#include "GL/Globals.hpp"
#include "Window/Window.hpp"

SSS_GL_BEGIN;

void createWindow(CreateArgs const& args)
{
    if (window)
        return;
    // Create window
    window.reset(new Window(args));
}

SSS_GL_API bool windowShouldClose() noexcept
{
    if (window)
        return window->shouldClose();
    return true;
}

void closeWindow()
{
    window.reset();
}

Shaders::Shared getPresetShaders(uint32_t id) noexcept
{
    return window->getPresetShaders(id);
}

void setRenderers(RendererBase::Vector const& renderers) noexcept
{
    window->setRenderers(renderers);
}
RendererBase::Vector const& getRenderers() noexcept
{
    return window->getRenderers();
}

void addRenderer(RendererBase::Shared renderer, size_t index)
{
    window->addRenderer(renderer, index);
}
void removeRenderer(RendererBase::Shared renderer)
{
    window->removeRenderer(renderer);
}

void drawRenderers()
{
    window->drawObjects();
}
void printBuffer()
{
    window->printFrame();
}
void saveScreenshot()
{
    window->saveScreenshot();
}

void blockInputs(int unblocking_key) noexcept
{
    window->blockInputs(unblocking_key);
}
void unblockInputs() noexcept
{
    window->unblockInputs();
}
bool areInputsBlocked() noexcept
{
    return window->areInputsBlocked();
}

void setInputStackTime(std::chrono::milliseconds ms)
{
    window->setInputStackTime(ms);
}

std::chrono::milliseconds getInputStackTime() noexcept
{
    return window->getInputStackTime();
}

KeyInputs const& getKeyInputs() noexcept
{
    return window->getKeyInputs();
}
MouseInputs const& getClickInputs() noexcept
{
    return window->getClickInputs();
}

void getCursorPos(int& x, int& y) noexcept
{
    window->getCursorPos(x, y);
}
void getCursorDiff(int& x, int& y) noexcept
{
    window->getCursorDiff(x, y);
}

ModelBase::Shared getHoveredModel() noexcept
{
    return window->getHovered();
}
ModelBase::Shared getClickedModel() noexcept
{
    return window->getClicked();
}
ModelBase::Shared getHeldModel() noexcept
{
    return window->getHeld();
}

long long getFPS() noexcept
{
    return window->getFPS();
}
void setFPSLimit(int fps_limit) noexcept
{
    window->setFPSLimit(fps_limit);
}
int getFPSLimit() noexcept
{
    return window->getFPSLimit();
}

void setVSYNC(bool state)
{
    window->setVSYNC(state);
}
bool getVSYNC() noexcept
{
    return window->getVSYNC();
}

void setTitle(std::string const& title)
{
    window->setTitle(title);
}
std::string getTitle() noexcept
{
    return window->getTitle();
}

void setSize(int w, int h)
{
    window->setDimensions(w, h);
}
void getSize(int& w, int& h) noexcept
{
    window->getDimensions(w, h);
}
float getSizeRatio() noexcept
{
    return window->getRatio();
}

void setPosition(int x0, int y0)
{
    window->setPosition(x0, y0);
}
void getPosition(int& x0, int& y0) noexcept
{
    window->getPosition(x0, y0);
}

void setFullscreen(bool fullscreen, int monitor_id)
{
    window->setFullscreen(fullscreen, monitor_id);
}
bool isFullscreen() noexcept
{
    return window->isFullscreen();
}

void setIconification(bool iconify)
{
    window->setIconification(iconify);
}
bool isIconified() noexcept
{
    return window->isIconified();
}

void setMaximization(bool maximize)
{
    window->setMaximization(maximize);
}
bool isMaximized() noexcept
{
    return window->isMaximized();
}

void setVisibility(bool show)
{
    window->setVisibility(show);
}
bool isVisible() noexcept
{
    return window->isVisible();
}

GLFWwindow* getGLFWwindow() noexcept
{
    return window->getGLFWwindow();
}
uint32_t maxGLSLTextureUnits() noexcept
{
    return window->maxGLSLTextureUnits();
}

SSS_GL_END;
