#include "JavaTabListPacket.h"

#include <vector>

#include "PacketListener.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "java/InputOutputStream/DataOutputStream.h"

JavaTabListPacket::JavaTabListPacket() {}

void JavaTabListPacket::read(DataInputStream* dis) {
    action = dis->readByte();
    if (action == ACTION_CLEAR) return;

    if (action == ACTION_BIND) entityId = dis->readInt();

    std::vector<uint8_t> raw(UUID_BYTES);
    dis->readFully(raw);
    uuid.assign(reinterpret_cast<const char*>(raw.data()), raw.size());

    if (action == ACTION_ADD) {
        name = readUtf(dis, MAX_NAME_LENGTH);
        ping = dis->readInt();
    }
}

void JavaTabListPacket::write(DataOutputStream* dos) {
    dos->writeByte(action);
    if (action == ACTION_CLEAR) return;

    if (action == ACTION_BIND) dos->writeInt(entityId);

    std::string id = uuid;
    id.resize(UUID_BYTES, '\0');
    std::vector<uint8_t> raw(id.begin(), id.end());
    dos->write(raw);

    if (action == ACTION_ADD) {
        writeUtf(name, dos);
        dos->writeInt(ping);
    }
}

void JavaTabListPacket::handle(PacketListener* listener) {
    listener->handleJavaTabList(shared_from_this());
}

int JavaTabListPacket::getEstimatedSize() {
    return 1 + 4 + UUID_BYTES + 2 + static_cast<int>(name.length()) * 2 + 4;
}
