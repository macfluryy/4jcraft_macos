#pragma once

#include <cstdint>

#if defined(__PS3__) || defined(__ORBIS__) || defined __PSVITA__
#define LOCALE_COUNT 21
#elif defined _XBOX_ONE
#define LOCALE_COUNT 19
#else
#define LOCALE_COUNT 11
#endif

class StringTable {
private:
    bool isStatic;

    std::unordered_map<std::wstring, std::wstring> m_stringsMap;
    std::vector<std::wstring> m_stringsVec;

    byteArray src;

public:
    // 	enum eLocale
    // 	{
    // 		eLocale_Default=0,
    // 		eLocale_American,
    // 		eLocale_Japanese,
    // 		eLocale_German,
    // 		eLocale_French,
    // 		eLocale_Spanish,
    // 		eLocale_Italian,
    // 		eLocale_Korean,
    // 		eLocale_TradChinese,
    // 		eLocale_Portuguese,
    // 		eLocale_Brazilian,
    // #if defined(__PS3__) || defined(__ORBIS__) || defined __PSVITA__
    // 		eLocale_Russian,
    // 		eLocale_Dutch,
    // 		eLocale_Finish,
    // 		eLocale_Swedish,
    // 		eLocale_Danish,
    // 		eLocale_Norwegian,
    // 		eLocale_Polish,
    // 		eLocale_Turkish,
    // 		eLocale_LatinAmericanSpanish,
    // 		eLocale_Greek,
    // #elif defined _XBOX_ONE || defined _XBOX
    // 		eLocale_British,
    // 		eLocale_Irish,
    // 		eLocale_Australian,
    // 		eLocale_NewZealand,
    // 		eLocale_Canadian,
    // 		eLocale_Mexican,
    // 		eLocale_FrenchCanadian,
    // 		eLocale_Austrian,
    // #endif
    // 	};

    StringTable(void);
    StringTable(std::uint8_t* pbData, unsigned int dataSize);
    ~StringTable(void);

    void getData(std::uint8_t** ppData, unsigned int* pSize);

    const wchar_t* getString(const std::wstring& id);
    const wchar_t* getString(int id);

    // static LPCWSTR m_wchLocaleCode[LOCALE_COUNT];

private:
    // std::wstring getLangId(DWORD dwLanguage=0);
};
