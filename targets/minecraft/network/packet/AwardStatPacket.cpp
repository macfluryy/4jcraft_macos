#include "AwardStatPacket.h"

#include <string.h>

#include "PacketListener.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "java/InputOutputStream/DataOutputStream.h"

AwardStatPacket::AwardStatPacket() { this->m_paramData.clear(); }

AwardStatPacket::AwardStatPacket(int statId, int count) {
    this->statId = statId;

    
    
    
    
    
    this->m_paramData.resize(sizeof(int));
    memcpy(this->m_paramData.data(), &count, sizeof(int));
}

AwardStatPacket::AwardStatPacket(int statId, std::vector<uint8_t>& paramData) {
    this->statId = statId;
    this->m_paramData = paramData;
}

AwardStatPacket::~AwardStatPacket() { m_paramData.clear(); }

void AwardStatPacket::handle(PacketListener* listener) {
    listener->handleAwardStat(shared_from_this());
    m_paramData.clear();
}

void AwardStatPacket::read(DataInputStream* dis)  
{
    statId = dis->readInt();

    
    int length = dis->readInt();
    if (length > 0) {
        m_paramData = std::vector<uint8_t>(length);
        dis->readFully(m_paramData);
    }
}

void AwardStatPacket::write(DataOutputStream* dos)  
{
    dos->writeInt(statId);
    dos->writeInt(m_paramData.size());
    if (m_paramData.size() > 0) dos->write(m_paramData);
}

int AwardStatPacket::getEstimatedSize() { return 6; }

bool AwardStatPacket::isAync() { return true; }


int AwardStatPacket::getCount() { return *((int*)this->m_paramData.data()); }


std::vector<uint8_t> AwardStatPacket::getParamData() { return m_paramData; }
