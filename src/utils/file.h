#pragma once

#include <string>

namespace Utils
{

/**
 * @brief Loads a shader from file into a std string.
 * @param shaderPath Path to file on disk.
 * @param shader Out variable that contains the loaded shader.
 * @return True on success, else false.
 */
bool loadShaderFromFile(const std::string &shaderPath, std::string &shader);

} // namespace Utils