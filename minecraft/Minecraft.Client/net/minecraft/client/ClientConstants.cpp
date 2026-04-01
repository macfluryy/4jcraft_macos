#include "ClientConstants.h"
#include "Minecraft.Client/net/minecraft/client/ClientConstants.h"
#include "Minecraft.Client/Common/src/BuildVer/BuildVer.h"

const std::wstring ClientConstants::VERSION_STRING =
    std::wstring(L"Minecraft Xbox ") + VER_FILEVERSION_STR_W +
    std::wstring(L" (4jcraft)");  //+ SharedConstants::VERSION_STRING;
