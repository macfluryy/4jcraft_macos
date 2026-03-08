#pragma once


#include "Packet.h"

class CustomPayloadPacket : public Packet, public std::enable_shared_from_this<CustomPayloadPacket>
{
public:

	// Mojang-defined custom packets
	static const std::wstring CUSTOM_BOOK_PACKET;
	static const std::wstring CUSTOM_BOOK_SIGN_PACKET;
	static const std::wstring TEXTURE_PACK_PACKET;
	static const std::wstring TRADER_LIST_PACKET;
	static const std::wstring TRADER_SELECTION_PACKET;
	static const std::wstring SET_ADVENTURE_COMMAND_PACKET;
	static const std::wstring SET_BEACON_PACKET;
	static const std::wstring SET_ITEM_NAME_PACKET;

	std::wstring identifier;
	int length;
	byteArray data;

	CustomPayloadPacket();
	CustomPayloadPacket(const std::wstring &identifier, byteArray data);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

public:
	static std::shared_ptr<Packet> create() { return std::shared_ptr<Packet>(new CustomPayloadPacket()); }
	virtual int getId() { return 250; }
};