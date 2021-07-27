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
		throw_exc(__FUNC_MSG(context_msg("Could not open file", filepath)));
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
		throw_exc(__FUNC_MSG(context_msg("Could not compile shader", &msg[0])));
	}
}

static GLuint loadShaders(std::string const& vertex_fp, std::string const& fragment_fp)
{
	// Read the shader files
	std::string const vertex_shader_code = readShaderFile(vertex_fp);
	std::string const fragment_shader_code = readShaderFile(fragment_fp);
	// Create the shaders
	GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
	// Compile the shaders
	compileShader(vertex_shader_id, vertex_shader_code);
	compileShader(fragment_shader_id, fragment_shader_code);

	// Link the program
	GLuint program_id = glCreateProgram();
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
		throw_exc(__FUNC_MSG(context_msg("Could not link program", &msg[0])));
	}

	// Free shaders
	glDetachShader(program_id, vertex_shader_id);
	glDetachShader(program_id, fragment_shader_id);
	glDeleteShader(vertex_shader_id);
	glDeleteShader(fragment_shader_id);

	// Return newly created & linked program
	return program_id;
}

Program::Program(std::weak_ptr<Window> window, std::string const& vertex_fp, std::string const& fragment_fp) try
	: _internal::WindowObject(window)
{
	Context const context(_window);
	_id = loadShaders(vertex_fp, fragment_fp);
}
__CATCH_AND_RETHROW_METHOD_EXC

void Program::use() const
{
	Context const context(_window);
	glUseProgram(_id);
}

Program::~Program()
{
	Context const context(_window);
	glDeleteProgram(_id);
}

// Return the location of a uniform variable for this program
GLuint Program::getUniformLocation(std::string const& name)
{
	Context const context(_window);
	return glGetUniformLocation(_id, name.c_str());
}

__SSS_GL_END