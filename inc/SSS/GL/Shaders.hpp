#pragma once

#include "_internal/basics.hpp"

__SSS_GL_BEGIN;

class Shaders : public _internal::WindowObject {
    friend class Window;
    friend class Renderer;

private:
    // Constructor : loads shaders and links them to a program
    Shaders(std::weak_ptr<Window> window);

public:
    ~Shaders();
    
    // Aliases
    using Ptr = std::unique_ptr<Shaders>;

    void loadFromFiles(std::string const& vertex_fp, std::string const& fragment_fp);
    void loadFromData(std::string const& vertex_data, std::string const& fragment_data);

    // Use this shader program for the current rendering
    void use() const;

    // Return the location of a uniform variable for this program
    GLint getUniformLocation(std::string const& name);

    void setUniform1iv(std::string const& name, GLsizei count, const GLint* value);
    void setUniformMat4fv(std::string const& name, GLsizei count,
        GLboolean transpose, GLfloat const* value);

private:
    bool _loaded{ false };
    // Program id
    GLuint _id{ 0 };
};

__SSS_GL_END;