#include <memory>

#include "Minecraft.World/net/minecraft/network/packet/CustomPayloadPacket.h"
#include "Minecraft.Client/net/minecraft/client/multiplayer/MultiPlayerLocalPlayer.h"
#include "Minecraft.Client/net/minecraft/client/multiplayer/ClientConnection.h"
#include "IUIScene_CommandBlockMenu.h"
#include "Minecraft.World/net/minecraft/world/level/tile/entity/CommandBlockEntity.h"
#include "java/InputOutputStream/ByteArrayOutputStream.h"
#include "java/InputOutputStream/DataOutputStream.h"
#include "Minecraft.Client/net/minecraft/client/Minecraft.h"

void IUIScene_CommandBlockMenu::Initialise(CommandBlockEntity* commandBlock) {
    m_commandBlock = commandBlock;
    SetCommand(m_commandBlock->getCommand());
}

void IUIScene_CommandBlockMenu::ConfirmButtonClicked() {
    ByteArrayOutputStream baos;
    DataOutputStream dos(&baos);

    dos.writeInt(m_commandBlock->x);
    dos.writeInt(m_commandBlock->y);
    dos.writeInt(m_commandBlock->z);
    dos.writeUTF(GetCommand());

    Minecraft::GetInstance()->localplayers[GetPad()]->connection->send(
        std::shared_ptr<CustomPayloadPacket>(new CustomPayloadPacket(
            CustomPayloadPacket::SET_ADVENTURE_COMMAND_PACKET,
            baos.toByteArray())));
}
