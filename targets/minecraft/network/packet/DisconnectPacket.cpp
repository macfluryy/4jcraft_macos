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

// Max chars of a custom disconnect string we accept on the wire. The screen
// wraps/clamps for display; this just bounds the readUtf so a bogus length
// can't be honoured.
static const int kMaxCustomDisconnectText = 256;

void DisconnectPacket::read(DataInputStream* dis)  // throws IOException
{
    reason = (eDisconnectReason)dis->readInt();
    // Only the new sentinel carries a trailing string, so vanilla LCE packets
    // (any other reason) read exactly as before - byte-for-byte compatible.
    if (reason == eDisconnect_CustomText) {
        m_customText = readUtf(dis, kMaxCustomDisconnectText);
    }
}

void DisconnectPacket::write(DataOutputStream* dos)  // throws IOException
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
        // writeUtf = short length + 2 bytes per char.
        size += 2 + (int)m_customText.length() * 2;
    }
    return size;
}

bool DisconnectPacket::canBeInvalidated() { return true; }

bool DisconnectPacket::isInvalidatedBy(std::shared_ptr<Packet> packet) {
    return true;
}