
#include "minecraft/SharedConstants.h"

#include <string>

const std::wstring SharedConstants::VERSION_STRING = L"1.6.4";
const bool SharedConstants::TEXTURE_LIGHTING = true;

std::wstring SharedConstants::readAcceptableChars() {
    
    
    std::wstring result =
        L" !\"#$%&'()*+,-./"
        L"0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_'"
        L"abcdefghijklmnopqrstuvwxyz{|}~"
        L" ÇüéâäàåçêëèïîìÄÅÉæÆôöòûùÿÖÜø£Ø×ƒáíóúñÑªº¿®¬½¼¡«»ã";
    return result;
}

bool SharedConstants::isAllowedChatCharacter(char ch) {
    
    
    
    return true;
}

std::wstring SharedConstants::acceptableLetters;

void SharedConstants::staticCtor() {
    acceptableLetters = readAcceptableChars();
}

const wchar_t
    SharedConstants::ILLEGAL_FILE_CHARACTERS[ILLEGAL_FILE_CHARACTERS_LENGTH] = {
        
        L'/', L'\n', L'\r', L'\t', L'\0', L'\f', L'`', L'?',
        L'*', L'\\', L'<',  L'>',  L'|',  L'\"', L':'};