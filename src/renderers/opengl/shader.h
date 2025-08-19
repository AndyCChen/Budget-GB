#pragma once

#include "glad/glad.h"

#include <string>

class Shader
{
  public:
	Shader(const std::string &pathToVertexShader, const std::string &pathTofragmentShader);

	Shader &operator=(Shader &&other) noexcept // move assignment operator
	{
		glDeleteProgram(m_shaderProgramID);

		m_shaderProgramID       = other.m_shaderProgramID;
		other.m_shaderProgramID = 0;

		return *this;
	}

	~Shader()
	{
		glDeleteProgram(m_shaderProgramID);
	}

	void useProgram() const
	{
		glUseProgram(m_shaderProgramID);
	}

	GLuint ID() const
	{
		return m_shaderProgramID;
	}

  private:
	GLuint m_shaderProgramID = 0;

	bool compileShader(GLuint shaderID, const std::string &pathToShader);
};
