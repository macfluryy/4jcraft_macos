#pragma once

class UITTFFont
{
private:
	PBYTE pbData;
	//DWORD dwDataSize;

public:
	UITTFFont(const std::string &path, S32 fallbackCharacter);
	~UITTFFont();
};
