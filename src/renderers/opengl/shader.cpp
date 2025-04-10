#include <fstream>
#include <sstream>

#include "fmt/base.h"
#include "shader.h"

Shader::Shader(std::string pathToVertexShader, const std::string &pathTofragmentShader)
{
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	if (compileShader(vertexShaderID, pathToVertexShader) && compileShader(fragmentShaderID, pathTofragmentShader))
	{
		m_shaderProgramID = glCreateProgram();

		glAttachShader(m_shaderProgramID, vertexShaderID);
		glAttachShader(m_shaderProgramID, fragmentShaderID);

		glLinkProgram(m_shaderProgramID);

		GLint status;
		constexpr GLsizei BUFFER_SIZE = 512;
		GLchar infoLog[BUFFER_SIZE];
		glGetProgramiv(m_shaderProgramID, GL_LINK_STATUS, &status);

		if (status == GL_FALSE)
		{
			glGetProgramInfoLog(m_shaderProgramID, BUFFER_SIZE, nullptr, infoLog);
			fmt::println(stderr, "Failed to link shaders {}\n{}\n{}", pathToVertexShader, pathTofragmentShader,
						 infoLog);

			// delete shader program when linking fails
			glDeleteProgram(m_shaderProgramID);
		}

		glDeleteShader(vertexShaderID);
		glDeleteShader(fragmentShaderID);
	}
}

bool Shader::loadShaderFromFile(const std::string &shaderPath, std::string &shader)
{
	std::ifstream file(shaderPath);
	if (!file.is_open())
	{
		fmt::println(stderr, "Failed to open shader: {}", shaderPath);
		return false;
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();

	shader = buffer.str();
	return true;
}

bool Shader::compileShader(GLuint shaderID, const std::string &pathToShader)
{
#ifdef USE_GL_VERSION_410
	std::string shaderVersion = "#version 410 core\n";
#else
	std::string shaderVersion = "#version 430 core\n";
#endif

	std::string shaderSource;

	if (!loadShaderFromFile(pathToShader, shaderSource))
		return false;

	const GLchar *sources[] = {shaderVersion.c_str(), shaderSource.c_str()};
	GLint sourceLengths[] = {static_cast<GLint>(shaderVersion.length()), static_cast<GLint>(shaderSource.length())};

	glShaderSource(shaderID, 2, sources, sourceLengths);
	glCompileShader(shaderID);

	GLint status;
	constexpr GLsizei BUFFER_SIZE = 512;
	GLchar infoLog[BUFFER_SIZE];
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE)
	{
		glGetShaderInfoLog(shaderID, BUFFER_SIZE, nullptr, infoLog);
		fmt::println(stderr, "Failed to compile {}\n{}", pathToShader, infoLog);
		return false;
	}

	return true;
}
