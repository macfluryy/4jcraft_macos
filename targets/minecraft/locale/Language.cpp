#include "Language.h"

#include <stdint.h>
#include <wchar.h>

#include <sstream>
#include <utility>
#include <vector>

#include "java/File.h"
#include "java/InputOutputStream/FileInputStream.h"
#include "java/InputOutputStream/InputStream.h"

// 4J - TODO - properly implement
// 4jcraft: done!

Language* Language::singleton = nullptr;

Language::Language() {
    // TODO: move the language loading out of the init to better match 1.3.x
    // see StringTranslate.java in MCP 7.x for more context
    File langFile(L"Common/res/lang/en_US.lang");
    if (langFile.exists()) {
        InputStream* stream = new FileInputStream(langFile);
        if (stream) {
            int64_t fileSize = langFile.length();
            if (fileSize > 0) {
                // 4jcraft: we would've used BufferedReader like the JE
                // equivalent here, but the lang file starts with a newline,
                // causing readLine() in this BufferedReader impl to just return
                // an empty string
                //
                // InputStreamReader reader(stream);
                // BufferedReader bufferedReader(&reader);
                // std::wstring line;
                std::vector<uint8_t> buffer((unsigned int)fileSize);
                int bytesRead = stream->read(buffer, 0, (unsigned int)fileSize);
                if (bytesRead > 0) {
                    std::string content(reinterpret_cast<char*>(buffer.data()),
                                        bytesRead);
                    std::istringstream iss(content);
                    std::string line;

                    // while (!(line = bufferedReader.readLine()).empty()) {
                    while (std::getline(iss, line)) {
                        size_t start = line.find_first_not_of(" \t\r\n");
                        if (start == std::string::npos) continue;

                        size_t end = line.find_last_not_of(" \t\r\n");
                        std::string trimmed =
                            line.substr(start, end - start + 1);

                        if (trimmed.empty() || trimmed[0] == '#') continue;

                        size_t equalsPos = trimmed.find('=');
                        if (equalsPos != std::string::npos) {
                            std::string key = trimmed.substr(0, equalsPos);
                            std::string value = trimmed.substr(equalsPos + 1);

                            std::wstring wkey(key.begin(), key.end());
                            std::wstring wvalue(value.begin(), value.end());

                            translateTable[wkey] = wvalue;
                        }
                    }
                }
            }
            delete stream;
        }
    }
}

Language* Language::getInstance() {
    // 4jcraft, fixes static init fiassco in I18n.cpp
    if (singleton == nullptr) {
        singleton = new Language();
    }

    return singleton;
}

/* 4J Jev, creates 2 identical functions.
std::wstring Language::getElement(const std::wstring& elementId)
{
        return elementId;
} */

// 4jcraft changed, again const reference into va_start, std forbids
std::wstring Language::getElement(std::wstring elementId, ...) {
    va_list args;
    va_start(args, elementId);
    std::wstring result = getElement(elementId, args);
    va_end(args);
    return result;
}

std::wstring Language::getElement(const std::wstring& elementId, va_list args) {
    auto it = translateTable.find(elementId);
    std::wstring formatString =
        (it != translateTable.end()) ? it->second : elementId;

    if (formatString.find(L'%') != std::wstring::npos) {
        int bufferSize = formatString.length() + 256;
        std::vector<wchar_t> buffer(bufferSize);

        int written =
            vswprintf(buffer.data(), bufferSize, formatString.c_str(), args);
        if (written >= 0) {
            return std::wstring(buffer.data(), written);
        }
    }

    return formatString;
}

std::wstring Language::getElementName(const std::wstring& elementId) {
    std::wstring nameKey = elementId + L".name";
    auto it = translateTable.find(nameKey);
    return (it != translateTable.end()) ? it->second : elementId;
}

std::wstring Language::getElementDescription(const std::wstring& elementId) {
    std::wstring descKey = elementId + L".description";
    auto it = translateTable.find(descKey);
    return (it != translateTable.end()) ? it->second : elementId;
}
