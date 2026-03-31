#include "Minecraft.World/Header Files/stdafx.h"
#include "LevelConflictException.h"

LevelConflictException::LevelConflictException(const std::wstring& msg)
    : RuntimeException(msg) {}