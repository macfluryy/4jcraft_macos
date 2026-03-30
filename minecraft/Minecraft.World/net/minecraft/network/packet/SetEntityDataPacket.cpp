#include "../../../../Header Files/stdafx.h"
#include <iostream>
#include "java/InputOutputStream/InputOutputStream.h"
#include "../../world/entity/net.minecraft.world.entity.h"
#include "PacketListener.h"
#include "SetEntityDataPacket.h"

SetEntityDataPacket::SetEntityDataPacket() {
    id = -1;
    packedItems = nullptr;
}

SetEntityDataPacket::~SetEntityDataPacket() { delete packedItems; }

SetEntityDataPacket::SetEntityDataPacket(
    int id, std::shared_ptr<SynchedEntityData> entityData, bool notJustDirty) {
    this->id = id;
    if (notJustDirty) {
        this->packedItems = entityData->getAll();
    } else {
        this->packedItems = entityData->packDirty();
    }
}

void SetEntityDataPacket::read(DataInputStream* dis)  // throws IOException
{
    id = dis->readInt();
    packedItems = SynchedEntityData::unpack(dis);
}

void SetEntityDataPacket::write(DataOutputStream* dos)  // throws IOException
{
    dos->writeInt(id);
    SynchedEntityData::pack(packedItems, dos);
}

void SetEntityDataPacket::handle(PacketListener* listener) {
    listener->handleSetEntityData(shared_from_this());
}

int SetEntityDataPacket::getEstimatedSize() { return 5; }

std::vector<std::shared_ptr<SynchedEntityData::DataItem> >*
SetEntityDataPacket::getUnpackedData() {
    return packedItems;
}
