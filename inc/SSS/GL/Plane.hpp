#pragma once

#include "_includes.hpp"
#include "_pointers.hpp"
#include "Shaders.hpp"

template<typename Enum>
struct EnableBitMaskOperators
{
    static const bool enable = false;
};

#define __ENABLE_BITMASK_OPERATORS(x)   \
template<>                              \
struct ::EnableBitMaskOperators<x>      \
{                                       \
    static const bool enable = true;    \
};

template<typename Enum>
typename std::enable_if<EnableBitMaskOperators<Enum>::enable, Enum>::type
operator| (Enum lhs, Enum rhs)
{
    using underlying = typename std::underlying_type<Enum>::type;
    return static_cast<Enum> (
        static_cast<underlying>(lhs)
        | static_cast<underlying>(rhs)
    );
}

template<typename Enum>
typename std::enable_if<EnableBitMaskOperators<Enum>::enable, Enum>::type
operator& (Enum lhs, Enum rhs)
{
    using underlying = typename std::underlying_type<Enum>::type;
    return static_cast<Enum> (
        static_cast<underlying>(lhs)
        & static_cast<underlying>(rhs)
    );
}

__SSS_GL_BEGIN

enum class Transformation {
    None = 0,
    Scaling = 1 << 0,
    Rotation = 1 << 1,
    Translation = 1 << 2,
    All = Scaling | Rotation | Translation,
};
__ENABLE_BITMASK_OPERATORS(Transformation);

class Plane {
private:

    static VAO::Ptr _vao;
    static VBO::Ptr _vbo;
    static IBO::Ptr _ibo;

public:
    Plane();
    Plane(std::string const& filepath);
    ~Plane();

    static void init();

    void scale(glm::vec3 scaling);
    void rotate(float radians, glm::vec3 axis);
    void translate(glm::vec3 translation);
    
    void resetTransformations(Transformation transformations);

    inline glm::mat4 getModelMat4() const noexcept { return _model; }
    void draw() const;

private:
    Texture _texture;

    glm::mat4 _og_scaling{ glm::scale(glm::mat4(1), glm::vec3(0)) };
    glm::mat4 _scaling{ _og_scaling };

    glm::mat4 _og_rotation{ glm::rotate(glm::mat4(1.), glm::radians(0.f), glm::vec3(0., 0., -1.)) };
    glm::mat4 _rotation{ _og_rotation };
    
    glm::mat4 _og_translation{ glm::translate(glm::mat4(1), glm::vec3(0)) };
    glm::mat4 _translation{ _og_translation };

    glm::mat4 _model{ _translation * _rotation * _scaling };
};

__SSS_GL_END

