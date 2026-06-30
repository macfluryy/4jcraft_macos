#include "ClientInformationPacket.h"

#include "PacketListener.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "java/InputOutputStream/DataOutputStream.h"

ClientInformationPacket::ClientInformationPacket() {
    this->shouldDelay = false;
    viewDistance = 0;
}

ClientInformationPacket::ClientInformationPacket(int viewDistance) {
    this->shouldDelay = false;
    this->viewDistance = viewDistance;
}

void ClientInformationPacket::read(DataInputStream* dis)  // throws IOException
{
    viewDistance = dis->readInt();
}

void ClientInformationPacket::write(DataOutputStream* dos)  // throws IOException
{
    dos->writeInt(viewDistance);
}

void ClientInformationPacket::handle(PacketListener* listener) {
    listener->handleClientInformation(shared_from_this());
}

int ClientInformationPacket::getEstimatedSize() { return sizeof(int); }
