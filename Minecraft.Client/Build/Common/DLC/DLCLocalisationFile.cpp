#include "../../../../Minecraft.World/Build/stdafx.h"
#include "DLCManager.h"
#include "DLCLocalisationFile.h"
#include "../../../Utils/StringTable.h"

DLCLocalisationFile::DLCLocalisationFile(const std::wstring &path) : DLCFile(DLCManager::e_DLCType_LocalisationData,path)
{	
	m_strings = NULL;
}

void DLCLocalisationFile::addData(PBYTE pbData, DWORD dwBytes)
{
	m_strings = new StringTable(pbData, dwBytes);
}