#include "../Platform/stdafx.h"
#include "../IO/Files/File.h"
#include "../IO/Streams/InputOutputStream.h"
#include "SharedConstants.h"

const std::wstring SharedConstants::VERSION_STRING = L"1.2.3";
const bool SharedConstants::TEXTURE_LIGHTING = true;

std::wstring SharedConstants::readAcceptableChars() {
    // 4J-PB - I've added ã in (for Portuguese in bed string) and added the
    // character at the same place in the default.png font
    std::wstring result =
        L" !\"#$%&'()*+,-./"
        L"0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_'"
        L"abcdefghijklmnopqrstuvwxyz{|}~"
        L" ÇüéâäàåçêëèïîìÄÅÉæÆôöòûùÿÖÜø£Ø×ƒáíóúñÑªº¿®¬½¼¡«»ã";
#if 0  // 4J - do we actually really need to get this from a file?
       // try {
       // BufferedReader br = new BufferedReader(new
       // InputStreamReader(SharedConstants.class.getResourceAsStream("/font.txt"),
       // "UTF-8"));
		BufferedReader *br = new BufferedReader(new InputStreamReader( new FileInputStream( new File( std::wstring(L"/font.txt") ) ) ) );
		std::wstring line;
		while ( !(line = br->readLine()).empty())
		{
			if (!( wcscmp( &line[0], L"#") == 0 ) )
			{
				result.append( line );
			}
		}
		br->close();
	//} catch (Exception e) {
		// TODO 4J Stu - Exception handling?
	//}
#endif
    return result;
}

bool SharedConstants::isAllowedChatCharacter(char ch) {
    // return ch != '§' && (acceptableLetters.indexOf(ch) >= 0 || (int) ch >
    // 32);
    //  4J Unused
    return true;
}

std::wstring SharedConstants::acceptableLetters =
    SharedConstants::readAcceptableChars();

void SharedConstants::staticCtor() {
    acceptableLetters = readAcceptableChars();
}

const wchar_t
    SharedConstants::ILLEGAL_FILE_CHARACTERS[ILLEGAL_FILE_CHARACTERS_LENGTH] = {
        // 4J-PB  - corrected
        L'/', L'\n', L'\r', L'\t', L'\0', L'\f', L'`', L'?',
        L'*', L'\\', L'<',  L'>',  L'|',  L'\"', L':'};