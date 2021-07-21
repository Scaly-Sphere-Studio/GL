#include "SSS/GL/Context.hpp"

__SSS_GL_BEGIN

std::vector<Context::Weak> Context::_instances{};
bool Context::LOG::constructor{ true };
bool Context::LOG::destructor{ true };
bool Context::LOG::glfw_init{ true };

Context::Context() try
{
    // Init GLFW
    if (_instances.empty()) {
        // Init GLFW
        glfwInit();
        if (LOG::glfw_init) {
            __LOG_OBJ_MSG("GLFW initialized.");
        }
        // Retrive monitors
        _internal::monitor_callback(nullptr, 0);
        glfwSetMonitorCallback(_internal::monitor_callback);
    }
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, DEBUGMODE ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    _hidden_window.reset(glfwCreateWindow(1, 1, "hidden window", nullptr, nullptr));
    glfwDefaultWindowHints();
}
__CATCH_AND_RETHROW_METHOD_EXC

Context::~Context()
{
    // Make context current if needed
    GLFWwindow* current_context = glfwGetCurrentContext();
    if (current_context == _hidden_window.get()) {
        cleanObjects();
    }
    else {
        std::lock_guard<std::mutex> const lock(_context_mutex);
        glfwMakeContextCurrent(_hidden_window.get());
        cleanObjects();
        glfwMakeContextCurrent(current_context);
    }
    cleanWeakPtrVector(_instances);
    if (_instances.empty()) {
        // Terminate GLFW
        glfwTerminate();
        if (LOG::glfw_init) {
            __LOG_OBJ_MSG("GLFW terminated.");
        }
    }
}

Context::Shared Context::create() try
{
    return (Shared)_instances.emplace_back(Shared(new Context()));
}
__CATCH_AND_RETHROW_FUNC_EXC

void Context::cleanObjects() noexcept
{
    ContextManager const context_manager(shared_from_this());
    _objects.windows.clear();
    _objects.models.classics.clear();
    _objects.models.planes.clear();
    _objects.models.buttons.clear();
    _objects.textures.classics.clear();
    _objects.textures.text.clear();
}

void Context::createWindow(uint32_t id, Window::Args const& args) try
{
    _objects.windows.try_emplace(id);
    _objects.windows.at(id).reset(
        new Window(shared_from_this(), _hidden_window.get(), args));
}
__CATCH_AND_RETHROW_METHOD_EXC

void Context::removeWindow(uint32_t id) try
{
    if (_objects.windows.count(id) != 0) {
        _objects.windows.erase(_objects.windows.find(id));
    }
}
__CATCH_AND_RETHROW_METHOD_EXC

void Context::removeWindowsThatShouldClose() try
{
    for (auto it = _objects.windows.cbegin(); it != _objects.windows.cend(); ) {
        if (it->second->shouldClose()) {
            it = _objects.windows.erase(it);
        }
        else {
            ++it;
        }
    }
}
__CATCH_AND_RETHROW_METHOD_EXC

Window::Ptr const& Context::getWindow(GLFWwindow* ptr) try
{
    for (Weak weak : _instances) {
        Shared context = weak.lock();
        if (!context) {
            continue;
        }
        for (auto it = context->_objects.windows.cbegin(); it != context->_objects.windows.cend(); ++it) {
            if (it->second->getGLFWwindow() == ptr) {
                return it->second;
            }
        }
    }
    throw_exc(ERR_MSG::NOTHING_FOUND);
}
__CATCH_AND_RETHROW_FUNC_EXC

void Context::createModel(ModelType type, uint32_t id) try
{
    switch (type) {
    case ModelType::Classic:
        _objects.models.classics.try_emplace(id);
        _objects.models.classics.at(id).reset(new Model(shared_from_this()));
        break;
    case ModelType::Plane:
        _objects.models.planes.try_emplace(id);
        _objects.models.planes.at(id).reset(new Plane(shared_from_this()));
        break;
    case ModelType::Button:
        _objects.models.buttons.try_emplace(id);
        _objects.models.buttons.at(id).reset(new Button(shared_from_this()));
        break;
    }
}
__CATCH_AND_RETHROW_METHOD_EXC

void Context::createTexture(TextureType type, uint32_t id) try
{
    switch (type) {
    case TextureType::Classic:
        _objects.textures.classics.try_emplace(id);
        _objects.textures.classics.at(id).reset(new Texture2D(shared_from_this()));
        break;
    case TextureType::Text:
        // TODO: Rework TextArea constructor
        _objects.textures.text.try_emplace(id);
        _objects.textures.text.at(id).reset(new TextTexture(shared_from_this(), 700, 700));
        break;
    }
}
__CATCH_AND_RETHROW_METHOD_EXC

void Context::pollTextureThreads() try
{
    // Loop over each Context instance
    for (Weak const& weak : _instances) {
        Shared context = weak.lock();
        if (!context) {
            continue;
        }
        // Loop over each Texture2D instance
        auto const& map = context->_objects.textures.classics;
        for (auto it = map.cbegin(); it != map.cend(); ++it) {
            Texture2D::Ptr const& tex = it->second;
            // If the loading thread is pending, edit the texture
            if (tex->_loading_thread.isPending()) {
                // Give the image to the OpenGL texture and notify all planes & buttons
                tex->_tex_w = tex->_loading_thread._w;
                tex->_tex_h = tex->_loading_thread._h;
                tex->_raw_pixels = std::move(tex->_loading_thread._pixels);
                tex->_raw_texture.edit(&tex->_raw_pixels[0], tex->_tex_w, tex->_tex_h);
                tex->_updatePlanesScaling();
                // Set thread as handled.
                tex->_loading_thread.setAsHandled();
            }
        }
    }
}
__CATCH_AND_RETHROW_FUNC_EXC

void Context::createShaders(uint32_t id, std::string const& vertex_fp, std::string const& fragment_fp)
{
    _objects.programs.try_emplace(id);
    _objects.programs.at(id).reset(
        new Program(shared_from_this(), vertex_fp, fragment_fp));
}

__SSS_GL_END