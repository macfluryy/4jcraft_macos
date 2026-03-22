#pragma once

class UITTFFont {
private:
    std::uint8_t* pbData;
    const std::string m_strFontName;

    // DWORD dwDataSize;

public:
    UITTFFont(const std::string& name, const std::string& path,
              S32 fallbackCharacter);
    ~UITTFFont();

    std::string getFontName();
};
