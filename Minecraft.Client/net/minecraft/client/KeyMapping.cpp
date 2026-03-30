#include "../../../../Minecraft.World/Header Files/stdafx.h"
#include "KeyMapping.h"

KeyMapping::KeyMapping(const std::wstring& name, int key) {
    this->name = name;
    this->key = key;
}