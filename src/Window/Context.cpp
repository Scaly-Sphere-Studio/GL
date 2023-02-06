#include "SSS/GL/Window.hpp"

SSS_GL_BEGIN;

void Context::_init(GLFWwindow* ptr)
{
    if (ptr == nullptr) {
        LOG_CTX_WRN(THIS_NAME, "given ptr is nullptr, are you sure this is intentional?");
    }
    _given = ptr;
    _previous = glfwGetCurrentContext();
    _equal = _given == _previous;
    if (!_equal) {
        glfwMakeContextCurrent(_given);
        // Log
        if (Log::GL::Context::query(Log::GL::Context::get().set_context)) {
            char buff[256];
            sprintf_s(buff, "Context -> make current: %p (given)", ptr);
            LOG_GL_MSG(buff);
        }
    }
}

Context::Context(std::weak_ptr<Window> ptr)
{
    Window::Shared const window = ptr.lock();
    if (!window) {
        return;
    }
    _init(window->getGLFWwindow());
}

Context::Context(GLFWwindow* ptr)
{
    _init(ptr);
}

Context::~Context()
{
    if (!_equal) {
        glfwMakeContextCurrent(_previous);
        // Log
        if (Log::GL::Context::query(Log::GL::Context::get().set_context)) {
            char buff[256];
            sprintf_s(buff, "Context -> make current: %p (previous)", _previous);
            LOG_GL_MSG(buff);
        }
    }
}

SSS_GL_END;