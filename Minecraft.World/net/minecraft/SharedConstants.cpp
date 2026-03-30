#include "../../Header Files/stdafx.h"
#include "../../ConsoleJavaLibs/File.h"
#include "../../ConsoleJavaLibs/InputOutputStream/InputOutputStream.h"
#include "SharedConstants.h"

const std::wstring SharedConstants::VERSION_STRING = L"1.6.4";
const bool SharedConstants::TEXTURE_LIGHTING = true;

std::wstring SharedConstants::readAcceptableChars() {
    // 4J-PB - I've added ã in (for Portuguese in bed string) and added the
    // character at the same place in the default.png font
    std::wstring result =
        L" !\"#$%&'()*+,-./"
        L"0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_'"
        L"abcdefghijklmnopqrstuvwxyz{|}~"
        L" ÇüéâäàåçêëèïîìÄÅÉæÆôöòûùÿÖÜø£Ø×ƒáíóúñÑªº¿®¬½¼¡«»ã";
    return result;
}

bool SharedConstants::isAllowedChatCharacter(char ch) {
    // return ch != '§' && (acceptableLetters.indexOf(ch) >= 0 || (int) ch >
    // 32);
    //  4J Unused
    return true;
}

std::wstring SharedConstants::acceptableLetters;

void SharedConstants::staticCtor() {
    acceptableLetters = readAcceptableChars();
}

const wchar_t
    SharedConstants::ILLEGAL_FILE_CHARACTERS[ILLEGAL_FILE_CHARACTERS_LENGTH] = {
        // 4J-PB  - corrected
        L'/', L'\n', L'\r', L'\t', L'\0', L'\f', L'`', L'?',
        L'*', L'\\', L'<',  L'>',  L'|',  L'\"', L':'};