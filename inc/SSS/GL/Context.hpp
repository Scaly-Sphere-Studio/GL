#pragma once

#include "_internal/includes.hpp"
#include "Window.hpp"
#include "Model.hpp"
#include "Plane.hpp"
#include "Button.hpp"
#include "Texture2D.hpp"
#include "TextTexture.hpp"

__SSS_GL_BEGIN

class Context :
    public _internal::ContextHolder,
    public std::enable_shared_from_this<Context>
{
    friend class ContextManager;
    friend class ContextLocker;

private:
    Context();                                      // Constructor
public:
    ~Context();                                     // Destructor
    Context(const Context&)             = delete;   // Copy constructor
    Context(Context&&)                  = delete;   // Move constructor
    Context& operator=(const Context&)  = delete;   // Copy assignment
    Context& operator=(Context&&)       = delete;   // Move assignment

    // --- Log options ---
    struct LOG {
        static bool constructor;
        static bool destructor;
        static bool glfw_init;
    };

private:
    using Weak = std::weak_ptr<Context>;
    static std::vector<Weak> _instances;

public:
    using Shared = std::shared_ptr<Context>;
    static Shared create();

public:
    // All context bound objects
    struct Objects {
        // Windows
        std::map<uint32_t, Window::Ptr> windows;
        // Models
        struct {
            std::map<uint32_t, Model::Ptr> classics;
            std::map<uint32_t, Plane::Ptr> planes;
            std::map<uint32_t, Button::Ptr> buttons;
        } models;
        // Textures
        struct {
            std::map<uint32_t, Texture2D::Ptr> classics;
            std::map<uint32_t, TextTexture::Ptr> text;
        } textures;

        // Rule of 5
        Objects()                           = default;  // Constructor
        ~Objects()                          = default;  // Destructor
        Objects(const Objects&)             = delete;   // Copy constructor
        Objects(Objects&&)                  = delete;   // Move constructor
        Objects& operator=(const Objects&)  = delete;   // Copy assignment
        Objects& operator=(Objects&&)       = delete;   // Move assignment
    };

private:
    Objects _objects;

public:
    inline Objects const& getObjects() const noexcept { return _objects; };
    void cleanObjects() noexcept;

    void createWindow(uint32_t id, Window::Args const& args);
    void removeWindow(uint32_t id);
    void removeWindowsThatShouldClose();
    static Window::Ptr const& getWindow(GLFWwindow* ptr);

    void createModel(ModelType type, uint32_t id);
    void removeModel(ModelType type, uint32_t id);

    void createTexture(TextureType type, uint32_t id);
    void removeTexture(TextureType type, uint32_t id);
    static void pollTextureThreads();
};

class ContextLocker {
public:
    ContextLocker()                                 = delete;   // Constructor
    ContextLocker(GLFWwindow const* ptr);                       // Constructor
    ~ContextLocker();                                           // Destructor
    ContextLocker(const ContextLocker&)             = delete;   // Copy constructor
    ContextLocker(ContextLocker&&)                  = delete;   // Move constructor
    ContextLocker& operator=(const ContextLocker&)  = delete;   // Copy assignment
    ContextLocker& operator=(ContextLocker&&)       = delete;   // Move assignment

private:
    GLFWwindow* _current;
    GLFWwindow* _previous;
    bool _equal;

    static std::mutex& getMutex(GLFWwindow* ptr);
};

__SSS_GL_END