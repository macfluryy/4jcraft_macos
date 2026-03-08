#pragma once


#include "Packet.h"

class UpdateGameRuleProgressPacket : public Packet, public std::enable_shared_from_this<UpdateGameRuleProgressPacket>
{
public:
	ConsoleGameRules::EGameRuleType m_definitionType;
	std::wstring m_messageId;
	int m_icon, m_auxValue;
	int m_dataTag;
	byteArray m_data;

	UpdateGameRuleProgressPacket();
	UpdateGameRuleProgressPacket(ConsoleGameRules::EGameRuleType definitionType, const std::wstring &messageId, int icon, int auxValue, int dataTag, void *data, int dataLength);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

public:
	static std::shared_ptr<Packet> create() { return std::shared_ptr<Packet>(new UpdateGameRuleProgressPacket()); }
	virtual int getId() { return 158; }
};