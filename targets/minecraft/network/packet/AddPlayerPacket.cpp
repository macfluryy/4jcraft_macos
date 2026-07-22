#include "AddPlayerPacket.h"

#include <vector>

#include "PacketListener.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "java/InputOutputStream/DataOutputStream.h"
#include "minecraft/world/entity/SyncedEntityData.h"
#include "minecraft/world/entity/player/Inventory.h"
#include "minecraft/world/entity/player/Player.h"
#include "minecraft/world/item/ItemInstance.h"

AddPlayerPacket::AddPlayerPacket() {
    id = -1;
    name = L"";
    x = 0;
    y = 0;
    z = 0;
    yRot = 0;
    xRot = 0;
    carriedItem = 0;
    xuid = INVALID_XUID;
    m_playerIndex = 0;
    m_skinId = 0;
    m_capeId = 0;
    m_uiGamePrivileges = 0;
    entityData = nullptr;
    unpack = nullptr;
}

AddPlayerPacket::~AddPlayerPacket() {
    if (unpack != nullptr) delete unpack;
}

AddPlayerPacket::AddPlayerPacket(std::shared_ptr<Player> player, PlayerUID xuid,
                                 PlayerUID OnlineXuid, int xp, int yp, int zp,
                                 int yRotp, int xRotp, int yHeadRotp) {
    id = player->entityId;
    name = player->getName();

    
    
    x = xp;  
    y = yp;  
    z = zp;  
    
    
    yRot = yRotp;
    xRot = xRotp;
    yHeadRot = yHeadRotp;  
    
    

    
    

    std::shared_ptr<ItemInstance> itemInstance =
        player->inventory->getSelected();
    carriedItem = itemInstance == nullptr ? 0 : itemInstance->id;

    this->xuid = xuid;
    this->OnlineXuid = OnlineXuid;
    m_playerIndex = (uint8_t)player->getPlayerIndex();
    m_skinId = player->getCustomSkin();
    m_capeId = player->getCustomCape();
    m_uiGamePrivileges = player->getAllPlayerGamePrivileges();

    entityData = player->getEntityData();
    unpack = nullptr;
}

void AddPlayerPacket::read(DataInputStream* dis)  
{
    id = dis->readInt();
    name = readUtf(dis, Player::MAX_NAME_LENGTH);
    x = dis->readInt();
    y = dis->readInt();
    z = dis->readInt();
    yRot = dis->readByte();
    xRot = dis->readByte();
    yHeadRot = dis->readByte();  
    carriedItem = dis->readShort();
    xuid = dis->readPlayerUID();
    OnlineXuid = dis->readPlayerUID();
    m_playerIndex = dis->readByte();
    int32_t skinId = dis->readInt();
    m_skinId = *(uint32_t*)&skinId;
    int32_t capeId = dis->readInt();
    m_capeId = *(uint32_t*)&capeId;
    int32_t privileges = dis->readInt();
    m_uiGamePrivileges = *(unsigned int*)&privileges;
    unpack = SynchedEntityData::unpack(dis);
}

void AddPlayerPacket::write(DataOutputStream* dos)  
{
    dos->writeInt(id);
    writeUtf(name, dos);
    dos->writeInt(x);
    dos->writeInt(y);
    dos->writeInt(z);
    dos->writeByte(static_cast<std::uint8_t>(yRot));
    dos->writeByte(static_cast<std::uint8_t>(xRot));
    dos->writeByte(static_cast<std::uint8_t>(yHeadRot));  
    dos->writeShort(carriedItem);
    dos->writePlayerUID(xuid);
    dos->writePlayerUID(OnlineXuid);
    dos->writeByte(static_cast<std::uint8_t>(m_playerIndex));
    dos->writeInt(m_skinId);
    dos->writeInt(m_capeId);
    dos->writeInt(m_uiGamePrivileges);
    entityData->packAll(dos);
}

void AddPlayerPacket::handle(PacketListener* listener) {
    listener->handleAddPlayer(shared_from_this());
}

int AddPlayerPacket::getEstimatedSize() {
    int iSize = sizeof(int) + Player::MAX_NAME_LENGTH + sizeof(int) +
                sizeof(int) + sizeof(int) + sizeof(uint8_t) + sizeof(uint8_t) +
                sizeof(short) + sizeof(PlayerUID) + sizeof(PlayerUID) +
                sizeof(int) + sizeof(uint8_t) + sizeof(unsigned int) +
                sizeof(uint8_t);

    if (entityData != nullptr) {
        iSize += entityData->getSizeInBytes();
    } else if (unpack != nullptr) {
        
        
        
    }

    return iSize;
}

std::vector<std::shared_ptr<SynchedEntityData::DataItem> >*
AddPlayerPacket::getUnpackedData() {
    if (unpack == nullptr) {
        unpack = entityData->getAll();
    }
    return unpack;
}
