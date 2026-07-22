#include "AddEntityPacket.h"

#include "PacketListener.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "java/InputOutputStream/DataOutputStream.h"
#include "minecraft/world/entity/Entity.h"

void AddEntityPacket::_init(std::shared_ptr<Entity> e, int type, int data,
                            int xp, int yp, int zp, int yRotp, int xRotp) {
    id = e->entityId;
    
    
    x = xp;  
    y = yp;  
    z = zp;  
    yRot = static_cast<uint8_t>(yRotp);
    xRot = static_cast<uint8_t>(xRotp);
    this->type = type;
    this->data = data;
    if (data > -1)  
                    
    {
        double xd = e->xd;
        double yd = e->yd;
        double zd = e->zd;
        double m = 3.9;
        if (xd < -m) xd = -m;
        if (yd < -m) yd = -m;
        if (zd < -m) zd = -m;
        if (xd > m) xd = m;
        if (yd > m) yd = m;
        if (zd > m) zd = m;
        xa = (int)(xd * 8000.0);
        ya = (int)(yd * 8000.0);
        za = (int)(zd * 8000.0);
    }
}

AddEntityPacket::AddEntityPacket() {}

AddEntityPacket::AddEntityPacket(std::shared_ptr<Entity> e, int type, int yRotp,
                                 int xRotp, int xp, int yp, int zp) {
    _init(e, type, -1, xp, yp, zp, yRotp,
          xRotp);  
                   
}

AddEntityPacket::AddEntityPacket(std::shared_ptr<Entity> e, int type, int data,
                                 int yRotp, int xRotp, int xp, int yp, int zp) {
    _init(e, type, data, xp, yp, zp, yRotp, xRotp);
}

void AddEntityPacket::read(DataInputStream* dis)  
                                                  
{
    id = dis->readShort();
    type = dis->readByte();
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
    data = dis->readInt();
    if (data > -1)  
                    
    {
        xa = dis->readShort();
        ya = dis->readShort();
        za = dis->readShort();
    }
}

void AddEntityPacket::write(
    DataOutputStream*
        dos)  
{
    dos->writeShort(id);
    dos->writeByte(static_cast<uint8_t>(type));
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
    dos->writeInt(data);
    if (data > -1)  
                    
    {
        dos->writeShort(xa);
        dos->writeShort(ya);
        dos->writeShort(za);
    }
}

void AddEntityPacket::handle(PacketListener* listener) {
    listener->handleAddEntity(shared_from_this());
}

int AddEntityPacket::getEstimatedSize() { return 11 + data > -1 ? 6 : 0; }
