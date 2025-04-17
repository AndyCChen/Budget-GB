#include "file.h"

#include <fstream>
#include <sstream>
#include "fmt/base.h"

bool Utils::loadShaderFromFile(const std::string &shaderPath, std::string &shader)
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
