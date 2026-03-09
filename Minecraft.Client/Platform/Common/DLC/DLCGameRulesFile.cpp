#include "../../Minecraft.World/Platform/stdafx.h"
#include "DLCManager.h"
#include "DLCGameRulesFile.h"

DLCGameRulesFile::DLCGameRulesFile(const std::wstring &path) : DLCGameRules(DLCManager::e_DLCType_GameRules,path)
{	
	m_pbData = NULL;
	m_dataBytes = 0;
}

void DLCGameRulesFile::addData(uint8_t *pbData, std::uint32_t dataBytes)
{
	m_pbData = pbData;
	m_dataBytes = dataBytes;
}

uint8_t *DLCGameRulesFile::getData(std::uint32_t &dataBytes)
{
	dataBytes = m_dataBytes;
	return m_pbData;
}
