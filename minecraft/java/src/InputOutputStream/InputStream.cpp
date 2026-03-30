#include "java/File.h"
#include "java/InputOutputStream/InputOutputStream.h"
#include "java/InputOutputStream/InputStream.h"

InputStream* InputStream::getResourceAsStream(const std::wstring& fileName) {
    File file(fileName);
    return file.exists() ? new FileInputStream(file) : nullptr;
}