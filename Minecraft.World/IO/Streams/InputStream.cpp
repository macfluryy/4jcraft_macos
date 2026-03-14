#include "../../Platform/stdafx.h"
#include "../Files/File.h"
#include "InputOutputStream.h"
#include "InputStream.h"

InputStream* InputStream::getResourceAsStream(const std::wstring& fileName) {
    return new FileInputStream(File(fileName));
}