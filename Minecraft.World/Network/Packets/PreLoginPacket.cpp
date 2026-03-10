#include "../../Platform/stdafx.h"
#include <cstdint>
#include <cstring>
#include <iostream>
#include "PacketListener.h"
#include "PreLoginPacket.h"
#include "../../IO/Streams/InputOutputStream.h"

PreLoginPacket::PreLoginPacket() 
{
	loginKey = L"";
	m_playerXuids = NULL;
	m_dwPlayerCount = 0;
	m_friendsOnlyBits = 0;
	m_ugcPlayersVersion = 0;
	ZeroMemory(m_szUniqueSaveName,m_iSaveNameLen);
	m_serverSettings = 0;
	m_hostIndex = 0;
	m_texturePackId = 0;
	m_netcodeVersion = 0;
}

PreLoginPacket::PreLoginPacket(std::wstring userName) 
{
	this->loginKey = userName;
	m_playerXuids = NULL;
	m_dwPlayerCount = 0;
	m_friendsOnlyBits = 0;
	m_ugcPlayersVersion = 0;
	ZeroMemory(m_szUniqueSaveName,m_iSaveNameLen);
	m_serverSettings = 0;
	m_hostIndex = 0;
	m_texturePackId = 0;
	m_netcodeVersion = 0;
}

PreLoginPacket::PreLoginPacket(std::wstring userName, PlayerUID *playerXuids, std::uint8_t playerCount, std::uint8_t friendsOnlyBits, std::uint32_t ugcPlayersVersion, const char *pszUniqueSaveName, std::uint32_t serverSettings, std::uint8_t hostIndex, std::uint32_t texturePackId) 
{
	this->loginKey = userName;
	m_playerXuids = playerXuids;
	m_dwPlayerCount = playerCount;
	m_friendsOnlyBits = friendsOnlyBits;
	m_ugcPlayersVersion = ugcPlayersVersion;
	memcpy(m_szUniqueSaveName,pszUniqueSaveName,m_iSaveNameLen);
	m_serverSettings = serverSettings;
	m_hostIndex = hostIndex;
	m_texturePackId = texturePackId;
	m_netcodeVersion = 0;
}

PreLoginPacket::~PreLoginPacket()
{
	if( m_playerXuids != NULL ) delete [] m_playerXuids;
}

void PreLoginPacket::read(DataInputStream *dis) //throws IOException 
{
	m_netcodeVersion = dis->readShort();

	loginKey = readUtf(dis, 32);

	m_friendsOnlyBits = static_cast<std::uint8_t>(dis->readByte());
	std::int32_t ugcPlayersVersion = dis->readInt();
	std::memcpy(&m_ugcPlayersVersion, &ugcPlayersVersion, sizeof(m_ugcPlayersVersion));
	m_dwPlayerCount = static_cast<std::uint8_t>(dis->readByte());
	if( m_dwPlayerCount > 0 )
	{
		m_playerXuids = new PlayerUID[m_dwPlayerCount];
		for(std::uint32_t i = 0; i < m_dwPlayerCount; ++i)
		{
			m_playerXuids[i] = dis->readPlayerUID();
		}
	}
	for(int i = 0; i < m_iSaveNameLen; ++i)
	{
		m_szUniqueSaveName[i] = static_cast<char>(dis->readByte());
	}
	std::int32_t serverSettings = dis->readInt();
	std::memcpy(&m_serverSettings, &serverSettings, sizeof(m_serverSettings));
	m_hostIndex = static_cast<std::uint8_t>(dis->readByte());
	
	std::int32_t texturePackId = dis->readInt();
	std::memcpy(&m_texturePackId, &texturePackId, sizeof(m_texturePackId));

	// Set the name of the map so we can check it for players banned lists
	app.SetUniqueMapName((char *)m_szUniqueSaveName);
}

void PreLoginPacket::write(DataOutputStream *dos) //throws IOException 
{
	dos->writeShort(MINECRAFT_NET_VERSION);

	writeUtf(loginKey, dos);
	
	dos->writeByte(m_friendsOnlyBits);
	std::int32_t ugcPlayersVersion = 0;
	std::memcpy(&ugcPlayersVersion, &m_ugcPlayersVersion, sizeof(m_ugcPlayersVersion));
	dos->writeInt(ugcPlayersVersion);
	dos->writeByte((std::uint8_t)m_dwPlayerCount);
	for(std::uint32_t i = 0; i < m_dwPlayerCount; ++i)
	{
		dos->writePlayerUID( m_playerXuids[i] );
	}

	app.DebugPrintf("*** PreLoginPacket::write - %s\n",m_szUniqueSaveName);
	for(int i = 0; i < m_iSaveNameLen; ++i)
	{
		dos->writeByte(static_cast<std::uint8_t>(m_szUniqueSaveName[i]));
	}
	std::int32_t serverSettings = 0;
	std::memcpy(&serverSettings, &m_serverSettings, sizeof(m_serverSettings));
	dos->writeInt(serverSettings);
	dos->writeByte(m_hostIndex);
	std::int32_t texturePackId = 0;
	std::memcpy(&texturePackId, &m_texturePackId, sizeof(texturePackId));
	dos->writeInt(texturePackId);
}

void PreLoginPacket::handle(PacketListener *listener) 
{
	listener->handlePreLogin(shared_from_this());
}

int PreLoginPacket::getEstimatedSize() 
{
	return 4 + 4 + (int)loginKey.length() + 4 +14 + 4 + 1 + 4;
}
