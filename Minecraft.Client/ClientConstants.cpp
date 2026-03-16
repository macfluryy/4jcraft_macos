#include "Platform/stdafx.h"
#include "ClientConstants.h"

const std::wstring ClientConstants::VERSION_STRING =
    std::wstring(L"Minecraft Xbox ") + VER_FILEVERSION_STR_W +
    std::wstring(L" (4jcraft)");  //+ SharedConstants::VERSION_STRING;
