#pragma once

#include "Shaders.hpp"

__SSS_GL_BEGIN;


class Renderer : public _internal::WindowObject {
protected:
    Renderer(std::weak_ptr<Window> window);             // Constructor
public:
    Renderer()                              = delete;   // Constructor (default)
    virtual ~Renderer()                     = default;  // Destructor
    Renderer(const Renderer&)               = delete;   // Copy constructor
    Renderer(Renderer&&)                    = delete;   // Move constructor
    Renderer& operator=(const Renderer&)    = delete;   // Copy assignment
    Renderer& operator=(Renderer&&)         = delete;   // Move assignment
    
    struct Chunk {
        std::string title;
        bool reset_depth_before{ false };
        bool use_camera{ true };
        uint32_t camera_ID{ 0 };
        std::deque<uint32_t> objects;
    };
    
    using Ptr = std::unique_ptr<Renderer>;

    std::deque<Chunk> chunks;
    std::string title;

protected:
    Shaders::Ptr _shaders;
    VAO::Ptr _vao;
    VBO::Ptr _vbo;
    IBO::Ptr _ibo;
public:
    virtual void render() = 0;

private:
    bool _is_active{ true };
public:
    inline void setActivity(bool state) noexcept { _is_active = state; };
    inline bool isActive() const noexcept { return _is_active; };
};

__SSS_GL_END;