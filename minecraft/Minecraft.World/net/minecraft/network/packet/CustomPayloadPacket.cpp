#include "../../../../Header Files/stdafx.h"
#include "java/InputOutputStream/InputOutputStream.h"
#include "PacketListener.h"
#include "CustomPayloadPacket.h"
#include <limits>

// Mojang-defined custom packets
const std::wstring CustomPayloadPacket::CUSTOM_BOOK_PACKET = L"MC|BEdit";
const std::wstring CustomPayloadPacket::CUSTOM_BOOK_SIGN_PACKET = L"MC|BSign";
const std::wstring CustomPayloadPacket::TEXTURE_PACK_PACKET = L"MC|TPack";
const std::wstring CustomPayloadPacket::TRADER_LIST_PACKET = L"MC|TrList";
const std::wstring CustomPayloadPacket::TRADER_SELECTION_PACKET = L"MC|TrSel";
const std::wstring CustomPayloadPacket::SET_ADVENTURE_COMMAND_PACKET =
    L"MC|AdvCdm";
const std::wstring CustomPayloadPacket::SET_BEACON_PACKET = L"MC|Beacon";
const std::wstring CustomPayloadPacket::SET_ITEM_NAME_PACKET = L"MC|ItemName";

CustomPayloadPacket::CustomPayloadPacket() {}

CustomPayloadPacket::CustomPayloadPacket(const std::wstring& identifier,
                                         byteArray data) {
    this->identifier = identifier;
    this->data = data;

    if (data.data != nullptr) {
        length = data.length;

        if (length > std::numeric_limits<short>::max()) {
            app.DebugPrintf("Payload may not be larger than 32K\n");
#ifndef _CONTENT_PACKAGE
            __debugbreak();
#endif
            // throw new IllegalArgumentException("Payload may not be larger
            // than 32k");
        }
    }
}

void CustomPayloadPacket::read(DataInputStream* dis) {
    identifier = readUtf(dis, 20);
    length = dis->readShort();

    if (length > 0 && length < std::numeric_limits<short>::max()) {
        if (data.data != nullptr) {
            delete[] data.data;
        }
        data = byteArray(length);
        dis->readFully(data);
    }
}

void CustomPayloadPacket::write(DataOutputStream* dos) {
    writeUtf(identifier, dos);
    dos->writeShort((short)length);
    if (data.data != nullptr) {
        dos->write(data);
    }
}

void CustomPayloadPacket::handle(PacketListener* listener) {
    listener->handleCustomPayload(shared_from_this());
}

int CustomPayloadPacket::getEstimatedSize() {
    return 2 + identifier.length() * 2 + 2 + length;
}
