#pragma once

#include "SSS/GL/_includes.hpp"

__SSS_GL_BEGIN

class Program {
public:
    // Aliases
    using Ptr = std::unique_ptr<Program>;

    // Constructor : loads shaders and links them to a program
    Program(std::string const& vertex_fp, std::string const& fragment_fp);
    ~Program();

    // Use this shader program for the current rendering
    inline void use() const noexcept { glUseProgram(_id); }

    // Return the location of a uniform variable for this program
    GLuint getUniformLocation(std::string const& name);

private:
    // Program id
    GLuint const _id;
};

__SSS_GL_END