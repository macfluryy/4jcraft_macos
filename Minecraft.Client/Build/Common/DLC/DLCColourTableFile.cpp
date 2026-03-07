#include "../../../../Minecraft.World/Build/stdafx.h"
#include "DLCManager.h"
#include "DLCColourTableFile.h"
#include "../../../Minecraft.h"
#include "../../../Textures/Packs/TexturePackRepository.h"
#include "../../../Textures/Packs/TexturePack.h"

DLCColourTableFile::DLCColourTableFile(const std::wstring &path) : DLCFile(DLCManager::e_DLCType_ColourTable,path)
{	
	m_colourTable = NULL;
}

DLCColourTableFile::~DLCColourTableFile()
{
	if(m_colourTable != NULL)
	{
		app.DebugPrintf("Deleting DLCColourTableFile data\n");
		delete m_colourTable;
	}
}

void DLCColourTableFile::addData(PBYTE pbData, DWORD dwBytes)
{
	ColourTable *defaultColourTable = Minecraft::GetInstance()->skins->getDefault()->getColourTable();
	m_colourTable = new ColourTable(defaultColourTable, pbData, dwBytes);
}