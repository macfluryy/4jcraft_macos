#include "Minecraft.World/Header Files/stdafx.h"

#include <string>

#include "java/File.h"
#include "Minecraft.World/ConsoleHelpers/StringHelpers.h"
#include "java/InputOutputStream/InputOutputStream.h"

#include "DLCManager.h"
#include "DLCGameRulesHeader.h"

DLCGameRulesHeader::DLCGameRulesHeader(const std::wstring& path)
    : DLCGameRules(DLCManager::e_DLCType_GameRulesHeader, path) {
    m_pbData = nullptr;
    m_dataBytes = 0;

    m_hasData = false;

    m_grfPath = path.substr(0, path.length() - 4) + L".grf";

    lgo = nullptr;
}

void DLCGameRulesHeader::addData(std::uint8_t* pbData,
                                 std::uint32_t dataBytes) {
    m_pbData = pbData;
    m_dataBytes = dataBytes;
}

std::uint8_t* DLCGameRulesHeader::getData(std::uint32_t& dataBytes) {
    dataBytes = m_dataBytes;
    return m_pbData;
}

void DLCGameRulesHeader::setGrfData(std::uint8_t* fData, std::uint32_t dataSize,
                                    StringTable* st) {
    if (!m_hasData) {
        m_hasData = true;

        // app.m_gameRules.loadGameRules(lgo, fData, fSize);

        app.m_gameRules.readRuleFile(lgo, fData, dataSize, st);
    }
}
