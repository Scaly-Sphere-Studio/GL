#pragma once

#include "_internal/pointers.hpp"
#include "Shaders.hpp"

__SSS_GL_BEGIN

struct RenderObject {
    RenderObject() = delete;
    RenderObject(uint32_t object_id_, uint32_t camera_id_)
        : object_id(object_id_), camera_id(camera_id_)
    {};
    uint32_t object_id;
    uint32_t camera_id;
};

class Renderer :
    public _internal::WindowObject,
    public std::deque<RenderObject>
{
public:
    using Ptr = std::unique_ptr<Renderer>;
    Renderer(std::weak_ptr<Window> window);             // Constructor
protected:
    Renderer()                              = delete;   // Constructor (default)
    ~Renderer()                             = default;  // Destructor
    Renderer(const Renderer&)               = delete;   // Copy constructor
    Renderer(Renderer&&)                    = delete;   // Move constructor
    Renderer& operator=(const Renderer&)    = delete;   // Copy assignment
    Renderer& operator=(Renderer&&)         = delete;   // Move assignment
public:
    virtual void render() const = 0;
protected:
    Shaders::Ptr _shaders;
    VAO::Ptr _vao;
};

class RenderRoutine : public std::deque<Renderer> {
public:
    void render() const;
};

__SSS_GL_END