#include "AddMobPacket.h"

#include <vector>

#include "PacketListener.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "java/InputOutputStream/DataOutputStream.h"
#include "minecraft/world/entity/EntityIO.h"
#include "minecraft/world/entity/LivingEntity.h"
#include "minecraft/world/entity/SyncedEntityData.h"

AddMobPacket::AddMobPacket() {
    id = -1;
    type = 0;
    x = 0;
    y = 0;
    z = 0;
    yRot = static_cast<uint8_t>(0);
    xRot = static_cast<uint8_t>(0);
    entityData = nullptr;
    unpack = nullptr;
}

AddMobPacket::~AddMobPacket() { delete unpack; }

AddMobPacket::AddMobPacket(std::shared_ptr<LivingEntity> mob, int yRotp,
                           int xRotp, int xp, int yp, int zp, int yHeadRotp) {
    id = mob->entityId;

    type = EntityIO::getId(mob);
    
    
    x = xp;  
    y = yp;  
    z = zp;  
    
    
    yRot = static_cast<uint8_t>(yRotp);
    xRot = static_cast<uint8_t>(xRotp);
    yHeadRot = static_cast<uint8_t>(yHeadRotp);
    
    
    

    
    double m = 3.9;
    double xd = mob->xd;
    double yd = mob->yd;
    double zd = mob->zd;
    if (xd < -m) xd = -m;
    if (yd < -m) yd = -m;
    if (zd < -m) zd = -m;
    if (xd > m) xd = m;
    if (yd > m) yd = m;
    if (zd > m) zd = m;
    this->xd = (int)(xd * 8000.0);
    this->yd = (int)(yd * 8000.0);
    this->zd = (int)(zd * 8000.0);

    

    entityData = mob->getEntityData();
    unpack = nullptr;
}

void AddMobPacket::read(DataInputStream* dis)  
{
    id = dis->readShort();
    type = static_cast<int>(dis->readByte()) & 0xff;
#ifdef _LARGE_WORLDS
    x = dis->readInt();
    y = dis->readInt();
    z = dis->readInt();
#else
    x = dis->readShort();
    y = dis->readShort();
    z = dis->readShort();
#endif
    yRot = dis->readByte();
    xRot = dis->readByte();
    yHeadRot = dis->readByte();
    xd = dis->readShort();
    yd = dis->readShort();
    zd = dis->readShort();
    unpack = SynchedEntityData::unpack(dis);
}

void AddMobPacket::write(DataOutputStream* dos)  
{
    dos->writeShort(id);
    dos->writeByte(static_cast<uint8_t>(type & 0xff));
#ifdef _LARGE_WORLDS
    dos->writeInt(x);
    dos->writeInt(y);
    dos->writeInt(z);
#else
    dos->writeShort(x);
    dos->writeShort(y);
    dos->writeShort(z);
#endif
    dos->writeByte(yRot);
    dos->writeByte(xRot);
    dos->writeByte(yHeadRot);
    dos->writeShort(xd);
    dos->writeShort(yd);
    dos->writeShort(zd);
    entityData->packAll(dos);
}

void AddMobPacket::handle(PacketListener* listener) {
    listener->handleAddMob(shared_from_this());
}

int AddMobPacket::getEstimatedSize() {
    int size = 11;
    if (entityData != nullptr) {
        size += entityData->getSizeInBytes();
    } else if (unpack != nullptr) {
        
        
        
    }
    return size;
}

std::vector<std::shared_ptr<SynchedEntityData::DataItem> >*
AddMobPacket::getUnpackedData() {
    if (unpack == nullptr) {
        unpack = entityData->getAll();
    }
    return unpack;
}
