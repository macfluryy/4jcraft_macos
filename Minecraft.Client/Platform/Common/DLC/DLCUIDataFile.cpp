#include "../../Minecraft.World/Platform/stdafx.h"
#include "DLCManager.h"
#include "DLCUIDataFile.h"

DLCUIDataFile::DLCUIDataFile(const std::wstring &path) : DLCFile(DLCManager::e_DLCType_UIData,path)
{	
	m_pbData = NULL;
	m_dataBytes = 0;
	m_canDeleteData = false;
}

DLCUIDataFile::~DLCUIDataFile()
{
	if(m_canDeleteData && m_pbData != NULL)
	{
		app.DebugPrintf("Deleting DLCUIDataFile data\n");
		delete [] m_pbData;
	}
}

void DLCUIDataFile::addData(std::uint8_t *pbData, std::uint32_t dataBytes,bool canDeleteData)
{
	m_pbData = pbData;
	m_dataBytes = dataBytes;
	m_canDeleteData = canDeleteData;
}

std::uint8_t *DLCUIDataFile::getData(std::uint32_t &dataBytes)
{
	dataBytes = m_dataBytes;
	return m_pbData;
}
