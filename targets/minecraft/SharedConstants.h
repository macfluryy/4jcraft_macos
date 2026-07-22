#pragma once

#include <string>

#include "java/Class.h"

class SharedConstants {
public:
    static void staticCtor();
    static const std::wstring VERSION_STRING;
    static inline constexpr int NETWORK_PROTOCOL_VERSION = 78;
    static const bool INGAME_DEBUG_OUTPUT = false;

    
    
    static inline constexpr int WORLD_RESOLUTION = 16;

    static bool isAllowedChatCharacter(char ch);

private:
    static std::wstring readAcceptableChars();

public:
    static inline constexpr int maxChatLength = 100;
    static std::wstring acceptableLetters;

    static inline constexpr int ILLEGAL_FILE_CHARACTERS_LENGTH = 15;
    static const wchar_t
        ILLEGAL_FILE_CHARACTERS[ILLEGAL_FILE_CHARACTERS_LENGTH];

    static const bool
        TEXTURE_LIGHTING;  
    static inline constexpr int TICKS_PER_SECOND = 20;

    static inline constexpr int FULLBRIGHT_LIGHTVALUE = 15 << 20 | 15 << 4;
};