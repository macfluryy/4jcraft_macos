#include "../../Minecraft.World/Platform/stdafx.h"
#include "DLCManager.h"
#include "DLCCapeFile.h"

DLCCapeFile::DLCCapeFile(const std::wstring &path) : DLCFile(DLCManager::e_DLCType_Cape,path)
{
}

void DLCCapeFile::addData(PBYTE pbData, DWORD dwBytes)
{
	app.AddMemoryTextureFile(m_path,pbData,dwBytes);
}