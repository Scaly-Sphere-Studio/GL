#ifndef SSS_GL_SHADERS_HPP
#define SSS_GL_SHADERS_HPP

#include "Basic.hpp"
#include "glm/glm.hpp"

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

// Ignore warning about STL exports as they're private members
#pragma warning(push, 2)
#pragma warning(disable: 4251)
#pragma warning(disable: 4275)

/** Non-exhaustive abstractization of \b OpenGL shaders.
 *  @sa Window::createShaders(), Renderer::setShadersID()
 */
class SSS_GL_API Shaders : public InstancedClass<Shaders> {
    friend SharedClass;

private:
    // Constructor 
    Shaders();

public:
    /** Destructor, unloads internal glProgram if needed.
     *  @sa Window::removeShaders()
     */
    ~Shaders();
    
    /** Internal preset shaders IDs*/
    enum class Preset : uint32_t {
        /** Plane shaders, used by Plane::Renderer by default.*/
        Plane,
        /** Line shaders, used by Line::Renderer by default.*/
        Line
    };

    using InstancedClass::create;
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



    /*
        UNIFORMS HANDLING
    */

    /** Simple handle to \c glGetUniformLocation().
     *  Context will always be accurately set.
     */
    GLint getUniformLocation(std::string const& name) const;

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

    /* Set bool uniform*/
    void setBool(const std::string& name, bool value) const;
    /* Set int uniform*/
    void setInt(const std::string& name, int value) const;
    /* Set float uniform*/
    void setFloat(const std::string& name, float value) const;

    /* VEC UNIFORMS*/
    /* Set vec2 uniform*/
    void setVec2(const std::string& name, const glm::vec2& value) const;  
    /* Set vec2 uniform*/
    void setVec2(const std::string& name, float x, float y) const;

    /* Set vec3 uniform*/
    void setVec3(const std::string& name, const glm::vec3& value) const;
    /* Set vec3 uniform*/
    void setVec3(const std::string& name, float x, float y, float z) const;

    /* Set vec4 uniform*/
    void setVec4(const std::string& name, const glm::vec4& value) const;
    /* Set vec4 uniform*/
    void setVec4(const std::string& name, float x, float y, float z, float w) const;

    /* Set mat2 uniform*/
    void setMat2(const std::string& name, const glm::mat2& mat) const;
    /* Set mat3 uniform*/
    void setMat3(const std::string& name, const glm::mat3& mat) const;
    /* Set mat4 uniform*/
    void setMat4(const std::string& name, const glm::mat4& mat) const;


private:
    bool _loaded{ false };
    // Program id
    GLuint _program_id{ 0 };
    // Shaders data
    std::string _vertex_data, _fragment_data;
};

#pragma warning(pop)

SSS_GL_END;

#endif // SSS_GL_SHADERS_HPP