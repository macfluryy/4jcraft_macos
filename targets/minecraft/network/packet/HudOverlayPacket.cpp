#include "HudOverlayPacket.h"

#include "PacketListener.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "java/InputOutputStream/DataOutputStream.h"

HudOverlayPacket::HudOverlayPacket() {}

HudOverlayPacket::HudOverlayPacket(int action, const std::wstring& text) {
    this->action = action;
    this->text = text;
}

void HudOverlayPacket::read(DataInputStream* dis) {
    action = dis->readByte();
    if (action == ACTION_ACTIONBAR || action == ACTION_TITLE ||
        action == ACTION_SUBTITLE) {
        text = readUtf(dis, MAX_TEXT_LENGTH);
    } else if (action == ACTION_TIMES) {
        fadeIn = dis->readInt();
        stay = dis->readInt();
        fadeOut = dis->readInt();
    }
}

void HudOverlayPacket::write(DataOutputStream* dos) {
    dos->writeByte(action);
    if (action == ACTION_ACTIONBAR || action == ACTION_TITLE ||
        action == ACTION_SUBTITLE) {
        writeUtf(text, dos);
    } else if (action == ACTION_TIMES) {
        dos->writeInt(fadeIn);
        dos->writeInt(stay);
        dos->writeInt(fadeOut);
    }
}

void HudOverlayPacket::handle(PacketListener* listener) {
    listener->handleHudOverlay(shared_from_this());
}

int HudOverlayPacket::getEstimatedSize() {
    return 1 + 2 + static_cast<int>(text.length()) * 2 + 12;
}
