#pragma once

#include "glad/glad.h"

#include <string>

class Shader
{
  public:
	Shader(const std::string &pathToVertexShader, const std::string &pathTofragmentShader);

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

	void reset(const std::string &pathToVertexShader, const std::string &pathTofragmentShader);

  private:
	GLuint m_shaderProgramID = 0;

	bool compileShader(GLuint shaderID, const std::string &pathToShader);

};
