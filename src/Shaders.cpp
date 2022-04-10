#include "SSS/GL/Shaders.hpp"
#include "SSS/GL/Window.hpp"

__SSS_GL_BEGIN

static std::string readShaderFile(std::string const& filepath)
{
	// Read the shader code from the file
	std::string shader_code;
	std::ifstream shader_stream(filepath, std::ios::in);
	// Throw if file could not be open
	if (!shader_stream.is_open()) {
		throw_exc(__FUNC_MSG(__CONTEXT_MSG("Could not open file", filepath)));
	}
	std::stringstream sstr;
	sstr << shader_stream.rdbuf();
	shader_code = sstr.str();
	shader_stream.close();
	// Return shader code
	return shader_code;
}

static void compileShader(GLuint shader_id, std::string const& shader_code)
{
	// Compile shader
	char const* c_str = shader_code.c_str();
	glShaderSource(shader_id, 1, &c_str, NULL);
	glCompileShader(shader_id);
	// Throw if failed
	GLint res;
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &res);
	if (res != GL_TRUE) {
		// Get error message
		int log_length;
		glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);
		std::vector<char> msg(log_length + 1);
		glGetShaderInfoLog(shader_id, log_length, NULL, &msg[0]);
		// Throw
		throw_exc(__FUNC_MSG(__CONTEXT_MSG("Could not compile shader", &msg[0])));
	}
}

static GLuint loadShaders(std::string const& vertex_data, std::string const& fragment_data)
{
	// Create the shaders
	GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
	if (vertex_shader_id == 0) {
		throw_exc(__CONTEXT_MSG("Could not create vertex shader", glGetError()));
	}
	GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
	if (fragment_shader_id == 0) {
		throw_exc(__CONTEXT_MSG("Could not create fragment shader", glGetError()));
	}
	// Compile the shaders
	compileShader(vertex_shader_id, vertex_data);
	compileShader(fragment_shader_id, fragment_data);

	// Link the program
	GLuint program_id = glCreateProgram();
	if (program_id == 0) {
		throw_exc(__CONTEXT_MSG("Could not create program", glGetError()));
	}
	glAttachShader(program_id, vertex_shader_id);
	glAttachShader(program_id, fragment_shader_id);
	glLinkProgram(program_id);
	// Throw if failed
	GLint res;
	glGetProgramiv(program_id, GL_LINK_STATUS, &res);
	if (res != GL_TRUE) {
		// Get error message
		int log_length;
		glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);
		std::vector<char> msg(log_length + 1);
		glGetProgramInfoLog(program_id, log_length, NULL, &msg[0]);
		// Throw
		throw_exc(__FUNC_MSG(__CONTEXT_MSG("Could not link program", &msg[0])));
	}

	// Free shaders
	glDetachShader(program_id, vertex_shader_id);
	glDetachShader(program_id, fragment_shader_id);
	glDeleteShader(vertex_shader_id);
	glDeleteShader(fragment_shader_id);

	// Return newly created & linked program
	return program_id;
}

Shaders::Shaders(std::weak_ptr<Window> window) try
	: _internal::WindowObject(window)
{
}
__CATCH_AND_RETHROW_METHOD_EXC

Shaders::~Shaders()
{
	if (!_loaded) {
		return;
	}
	Context const context(_window);
	glDeleteProgram(_id);
}

void Shaders::loadFromFiles(std::string const& vertex_fp, std::string const& fragment_fp) try
{
	loadFromData(readShaderFile(vertex_fp), readShaderFile(fragment_fp));
}
__CATCH_AND_RETHROW_METHOD_EXC

void Shaders::loadFromData(std::string const& vertex_data, std::string const& fragment_data) try
{
	Context const context(_window);
	_id = loadShaders(vertex_data, fragment_data);
	_loaded = true;
}
__CATCH_AND_RETHROW_METHOD_EXC

void Shaders::use() const
{
	if (!_loaded) {
		__LOG_OBJ_METHOD_WRN("Shaders were not loaded!");
		return;
	}
	Context const context(_window);
	glUseProgram(_id);
}

// Return the location of a uniform variable for this program
GLint Shaders::getUniformLocation(std::string const& name)
{
	Context const context(_window);
	return glGetUniformLocation(_id, name.c_str());
}

void Shaders::setUniform1iv(std::string const& name, GLsizei count, const GLint* value)
{
	glUniform1iv(getUniformLocation(name), count, value);
}

void Shaders::setUniformMat4fv(std::string const& name, GLsizei count,
	GLboolean transpose, GLfloat const* value)
{
	glUniformMatrix4fv(getUniformLocation(name), count, transpose, value);
}

__SSS_GL_END