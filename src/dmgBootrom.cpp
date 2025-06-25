#include "dmgBootrom.h"

#include <fstream>

bool DmgBootRom::loadFromFile(const std::string &path)
{
	std::ifstream file(path, std::ios::binary);
	if (!file.is_open())
	{
		m_errorMsg = "Failed to open bootrom at path";
		loaded     = false;
		return loaded;
	}

	file.seekg(0, std::ios::end);
	std::size_t fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	if (fileSize != m_bootrom.size())
	{
		m_errorMsg = "Bootrom size is invalid, make sure it is a 256k DMG bootrom!";
		loaded     = false;
		return loaded;
	}

	file.read(reinterpret_cast<char *>(m_bootrom.data()), fileSize);
	file.close();

	loaded = true;
	return loaded;
}