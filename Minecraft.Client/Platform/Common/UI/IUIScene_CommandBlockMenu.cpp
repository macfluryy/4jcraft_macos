#include "../../stdafx.h"
#include "../../../../Minecraft.World/Network/Packets/CustomPayloadPacket.h"
#include "../../../Player/MultiPlayerLocalPlayer.h"
#include "../../../Network/ClientConnection.h"
#include "IUIScene_CommandBlockMenu.h"

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
