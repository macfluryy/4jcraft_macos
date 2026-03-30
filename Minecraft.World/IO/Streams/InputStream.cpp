#include "../../Platform/stdafx.h"
#include "../Files/File.h"
#include "InputOutputStream.h"
#include "InputStream.h"

InputStream* InputStream::getResourceAsStream(const std::wstring& fileName) {
    File file(fileName);
    return file.exists() ? new FileInputStream(file) : nullptr;
}