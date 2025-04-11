#pragma once

#include "glad/glad.h"

#include <string>

class Shader
{
  public:
	Shader(std::string pathToVertexShader, const std::string &pathTofragmentShader);

	~Shader()
	{
		glDeleteProgram(m_shaderProgramID);
	}

	void useProgram()
	{
		glUseProgram(m_shaderProgramID);
	}

  private:
	GLuint m_shaderProgramID;

	bool loadShaderFromFile(const std::string &shaderPath, std::string &shader);
	bool compileShader(GLuint shaderID, const std::string &pathToShader);
};
