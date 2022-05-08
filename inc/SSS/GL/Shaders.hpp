#pragma once

#include "Basics.hpp"

/** @file
 *  Defines class SSS::GL::Shaders.
 */

namespace SSS::Log::GL {
    /** Logging properties for SSS::GL::Shaders.*/
    struct Shaders : public LogBase<Shaders> {
        using LOG_STRUCT_BASICS(Log, Shaders);
        bool life_state = false;
        bool loading = false;
    };
}

SSS_GL_BEGIN;

/** Non-exhaustive abstractization of \b OpenGL shaders.
 *  @sa Window::createShaders(), Renderer::setShadersID()
 */
class Shaders : public _internal::WindowObject {
    friend class Window;
    friend class Renderer;

private:
    // Constructor 
    Shaders(std::weak_ptr<Window> window);

public:
    /** Destructor, unloads internal glProgram if needed.
     *  @sa Window::removeShaders(), Window::cleanObjects().
     */
    ~Shaders();
    
    /** Unique ptr stored in Window objects.*/
    using Ptr = std::unique_ptr<Shaders>;
    /** Internal preset shaders IDs*/
    enum class Preset : uint32_t {
        /** Indicates that further ID values are reserved.*/
        First = 0x80000000,
        /** Plane shaders, used by Plane::Renderer by default.*/
        Plane
    };

    /** Loads shaders from raw strings (useful for Preset shaders).
     *  Context will always be accurately set.
     *  @sa loadFromFiles()
     */
    void loadFromStrings(std::string const& vertex_data, std::string const& fragment_data);
    /** Loads shaders from filepaths.
     *  Context will always be accurately set.
     *  @sa loadFromStrings()
     */
    void loadFromFiles(std::string const& vertex_fp, std::string const& fragment_fp);

    /** Simple handle to \c glUseProgram().
     *  Context will always be accurately set.
     */
    void use() const;

    /** Simple handle to \c glGetUniformLocation().
     *  Context will always be accurately set.
     */
    GLint getUniformLocation(std::string const& name);

    /** Simple handle to \c glUniform1iv().
     *  Context will always be accurately set.
     */
    void setUniform1iv(std::string const& name, GLsizei count, const GLint* value);
    /** Simple handle to \c glUniformMatrix4fv().
     *  Context will always be accurately set.
     */
    void setUniformMat4fv(std::string const& name, GLsizei count,
        GLboolean transpose, GLfloat const* value);

private:
    bool _loaded{ false };
    // Program id
    GLuint _id{ 0 };
};

SSS_GL_END;