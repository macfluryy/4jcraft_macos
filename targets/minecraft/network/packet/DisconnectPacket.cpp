#include "minecraft/network/packet/DisconnectPacket.h"

#include <stdio.h>

#include <memory>

#include "PacketListener.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "java/InputOutputStream/DataOutputStream.h"

DisconnectPacket::DisconnectPacket() { reason = eDisconnect_None; }

DisconnectPacket::DisconnectPacket(eDisconnectReason reason) {
    this->reason = reason;
}

DisconnectPacket::DisconnectPacket(const std::wstring& customText) {
    this->reason = eDisconnect_CustomText;
    this->m_customText = customText;
}




static const int kMaxCustomDisconnectText = 256;

void DisconnectPacket::read(DataInputStream* dis)  
{
    reason = (eDisconnectReason)dis->readInt();
    
    
    if (reason == eDisconnect_CustomText) {
        m_customText = readUtf(dis, kMaxCustomDisconnectText);
    }
}

void DisconnectPacket::write(DataOutputStream* dos)  
{
    dos->writeInt((int)reason);
    if (reason == eDisconnect_CustomText) {
        writeUtf(m_customText, dos);
    }
}

void DisconnectPacket::handle(PacketListener* listener) {
    listener->handleDisconnect(shared_from_this());
}

int DisconnectPacket::getEstimatedSize() {
    int size = sizeof(eDisconnectReason);
    if (reason == eDisconnect_CustomText) {
        
        size += 2 + (int)m_customText.length() * 2;
    }
    return size;
}

bool DisconnectPacket::canBeInvalidated() { return true; }

bool DisconnectPacket::isInvalidatedBy(std::shared_ptr<Packet> packet) {
    return true;
}