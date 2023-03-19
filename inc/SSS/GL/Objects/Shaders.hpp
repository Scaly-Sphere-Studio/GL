#ifndef SSS_GL_SHADERS_HPP
#define SSS_GL_SHADERS_HPP

#include "SSS/GL/Objects/Basic.hpp"

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
class Shaders final : public Basic::SharedBase<Shaders> {
    friend class Basic::SharedBase<Shaders>;
    friend class Window;

private:
    // Constructor 
    Shaders(std::shared_ptr<Window> window);

public:
    /** Destructor, unloads internal glProgram if needed.
     *  @sa Window::removeShaders()
     */
    ~Shaders();
    
    /** Internal preset shaders IDs*/
    enum class Preset : uint32_t {
        /** Plane shaders, used by Plane::Renderer by default.*/
        Plane
    };

    using SharedBase::create;
    static Shared create(std::string const& vert_file, std::string const& frag_file);

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

    inline std::string getVertexData() const noexcept { return _vertex_data; };
    inline std::string getFragmentData() const noexcept { return _fragment_data; };

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
    /** Simple handle to \c glUniform1fv().
     *  Context will always be accurately set.
     */
    void setUniform1fv(std::string const& name, GLsizei count, const GLfloat* value);
    /** Simple handle to \c glUniformMatrix4fv().
     *  Context will always be accurately set.
     */
    void setUniformMat4fv(std::string const& name, GLsizei count,
        GLboolean transpose, GLfloat const* value);

private:
    bool _loaded{ false };
    // Program id
    GLuint _program_id{ 0 };
    // Shaders data
    std::string _vertex_data, _fragment_data;
};

SSS_GL_END;

#endif // SSS_GL_SHADERS_HPP