#include "../../Minecraft.World/Platform/stdafx.h"

#include <string>

#include "../../Minecraft.World/IO/Files/File.h"
#include "../../Minecraft.World/Util/StringHelpers.h"
#include "../../Minecraft.World/IO/Streams/InputOutputStream.h"

#include "DLCManager.h"
#include "DLCGameRulesHeader.h"

DLCGameRulesHeader::DLCGameRulesHeader(const std::wstring &path) : DLCGameRules(DLCManager::e_DLCType_GameRulesHeader,path)
{	
	m_pbData = NULL;
	m_dataBytes = 0;

	m_hasData = false;

	m_grfPath = path.substr(0, path.length() - 4) + L".grf";

	lgo = NULL;
}

void DLCGameRulesHeader::addData(uint8_t *pbData, std::uint32_t dataBytes)
{
	m_pbData = pbData;
	m_dataBytes = dataBytes;


#if 0
	byteArray data(m_pbData, m_dataBytes);
	ByteArrayInputStream bais(data);
	DataInputStream dis(&bais);

	// Init values.
	int version_number;
	byte compression_type;
	std::wstring texturepackid;

	// Read Datastream.
	version_number = dis.readInt();
	compression_type = dis.readByte();
	m_defaultSaveName = dis.readUTF();
	m_displayName = dis.readUTF();
	texturepackid = dis.readUTF();
	m_grfPath = dis.readUTF();

	// Debug printout.
	app.DebugPrintf	(
						"DLCGameRulesHeader::readHeader:\n"
						"\tversion_number = '%d',\n"
						"\tcompression_type = '%d',\n"
						"\tdefault_savename = '%s',\n"
						"\tdisplayname = '%s',\n"
						"\ttexturepackid = '%s',\n"
						"\tgrf_path = '%s',\n",

						version_number, compression_type,

						wstringtofilename(m_defaultSaveName),
						wstringtofilename(m_displayName),
						wstringtofilename(texturepackid),
						wstringtofilename(m_grfPath)
					);

	// Texture Pack.
	m_requiredTexturePackId = _fromString<long>(texturepackid);
	m_bRequiresTexturePack = m_requiredTexturePackId > 0;

	dis.close();
	bais.close();
	bais.reset();
#endif
}

uint8_t *DLCGameRulesHeader::getData(std::uint32_t &dataBytes)
{
	dataBytes = m_dataBytes;
	return m_pbData;
}

void DLCGameRulesHeader::setGrfData(uint8_t *fData, std::uint32_t dataSize, StringTable *st)
{
	if (!m_hasData)
	{
		m_hasData = true;
	
		//app.m_gameRules.loadGameRules(lgo, fData, fSize);

		app.m_gameRules.readRuleFile(lgo, fData, dataSize, st);
	}
}
