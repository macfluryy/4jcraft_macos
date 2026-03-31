#include <limits>

#include "PacketListener.h"
#include "CustomPayloadPacket.h"
#include "Minecraft.Client/Linux/Linux_App.h"
#include "Minecraft.Client/Linux/Stubs/winapi_stubs.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "java/InputOutputStream/DataOutputStream.h"

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
                                         std::vector<uint8_t> data) {
    this->identifier = identifier;
    this->data = data;

    if (!data.empty()) {
        length = data.size();

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
        data = std::vector<uint8_t>(length);
        dis->readFully(data);
    }
}

void CustomPayloadPacket::write(DataOutputStream* dos) {
    writeUtf(identifier, dos);
    dos->writeShort((short)length);
    if (!data.empty()) {
        dos->write(data);
    }
}

void CustomPayloadPacket::handle(PacketListener* listener) {
    listener->handleCustomPayload(shared_from_this());
}

int CustomPayloadPacket::getEstimatedSize() {
    return 2 + identifier.length() * 2 + 2 + length;
}
