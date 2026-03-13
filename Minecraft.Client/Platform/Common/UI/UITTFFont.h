#pragma once

#include <cstdint>

class UITTFFont
{
private:
	std::uint8_t *pbData;
	//DWORD dwDataSize;

public:
	UITTFFont(const std::string &path, S32 fallbackCharacter);
	~UITTFFont();
};
