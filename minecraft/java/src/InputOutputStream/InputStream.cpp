#include "File.h"
#include "InputOutputStream/InputOutputStream.h"
#include "InputOutputStream/InputStream.h"

InputStream* InputStream::getResourceAsStream(const std::wstring& fileName) {
    File file(fileName);
    return file.exists() ? new FileInputStream(file) : nullptr;
}