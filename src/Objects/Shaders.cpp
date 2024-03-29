#include "GL/Objects/Shaders.hpp"

SSS_GL_BEGIN;

Shaders::Shaders() try
{
	// Log
	if (Log::GL::Shaders::query(Log::GL::Shaders::get().life_state)) {
		LOG_GL_MSG("Shaders -> created");
	}
}
CATCH_AND_RETHROW_METHOD_EXC;

Shaders::~Shaders()
{
	if (!_loaded) {
		// Log
		if (Log::GL::Shaders::query(Log::GL::Shaders::get().life_state)) {
			LOG_GL_MSG("Shaders -> deleted (wasn't loaded)");
		}
		return;
	}
	glDeleteProgram(_program_id);

	// Log
	if (Log::GL::Shaders::query(Log::GL::Shaders::get().life_state)) {
		LOG_GL_MSG("Shaders -> deleted (id: " + toString(_program_id) + ")");
	}
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
		throw_exc(FUNC_MSG(CONTEXT_MSG("Could not compile shader", &msg[0])));
	}
}

static GLuint loadShaders(std::string const& vertex_data, std::string const& fragment_data)
{
	// Create the shaders
	GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
	if (vertex_shader_id == 0) {
		throw_exc(CONTEXT_MSG("Could not create vertex shader", glGetError()));
	}
	GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
	if (fragment_shader_id == 0) {
		throw_exc(CONTEXT_MSG("Could not create fragment shader", glGetError()));
	}
	// Compile the shaders
	compileShader(vertex_shader_id, vertex_data);
	compileShader(fragment_shader_id, fragment_data);

	// Link the program
	GLuint program_id = glCreateProgram();
	if (program_id == 0) {
		throw_exc(CONTEXT_MSG("Could not create program", glGetError()));
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
		throw_exc(FUNC_MSG(CONTEXT_MSG("Could not link program", &msg[0])));
	}

	// Free shaders
	glDetachShader(program_id, vertex_shader_id);
	glDetachShader(program_id, fragment_shader_id);
	glDeleteShader(vertex_shader_id);
	glDeleteShader(fragment_shader_id);

	// Return newly created & linked program
	return program_id;
}

Shaders::Shared Shaders::create(std::string const& vertex_fp, std::string const& fragment_fp)
{
	Shaders::Shared shader = create();
	shader->loadFromFiles(vertex_fp, fragment_fp);
	return shader;
}

void Shaders::loadFromStrings(std::string const& vertex_data, std::string const& fragment_data) try
{
	_program_id = loadShaders(vertex_data, fragment_data);
	_vertex_data = vertex_data;
	_fragment_data = fragment_data;
	_loaded = true;

	// Log
	if (Log::GL::Shaders::query(Log::GL::Shaders::get().loading)) {
		LOG_GL_MSG("Shaders -> loaded");
	}
}
CATCH_AND_RETHROW_METHOD_EXC;

void Shaders::loadFromFiles(std::string const& vertex_fp, std::string const& fragment_fp) try
{
	loadFromStrings(readFile(vertex_fp), readFile(fragment_fp));
}
CATCH_AND_RETHROW_METHOD_EXC;

void Shaders::use() const
{
	if (!_loaded) {
		LOG_METHOD_WRN("Shaders were not loaded!");
		return;
	}
	glUseProgram(_program_id);
}

// Return the location of a uniform variable for this program
GLint Shaders::getUniformLocation(std::string const& name)
{
	return glGetUniformLocation(_program_id, name.c_str());
}

void Shaders::setUniform1iv(std::string const& name, GLsizei count, const GLint* value)
{
	glUniform1iv(getUniformLocation(name), count, value);
}

void Shaders::setUniform1fv(std::string const& name, GLsizei count, const GLfloat* value)
{
	glUniform1fv(getUniformLocation(name), count, value);
}

void Shaders::setUniformMat4fv(std::string const& name, GLsizei count,
	GLboolean transpose, GLfloat const* value)
{
	glUniformMatrix4fv(getUniformLocation(name), count, transpose, value);
}

SSS_GL_END;