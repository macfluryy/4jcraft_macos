#include "../../../../Header Files/stdafx.h"
#include "../net.minecraft.commands.h"
#include "../../world/entity/item/net.minecraft.world.entity.item.h"
#include "../../world/item/net.minecraft.world.item.h"
#include "../../network/packet/net.minecraft.network.packet.h"
#include "../../Minecraft.Client/net/minecraft/server/level/ServerPlayer.h"
#include "GiveItemCommand.h"

EGameCommand GiveItemCommand::getId() { return eGameCommand_Give; }

int GiveItemCommand::getPermissionLevel() { return LEVEL_GAMEMASTERS; }

void GiveItemCommand::execute(std::shared_ptr<CommandSender> source,
                              byteArray commandData) {
    ByteArrayInputStream bais(commandData);
    DataInputStream dis(&bais);

    PlayerUID uid = dis.readPlayerUID();
    int item = dis.readInt();
    int amount = dis.readInt();
    int aux = dis.readInt();
    std::wstring tag = dis.readUTF();

    bais.reset();

    std::shared_ptr<ServerPlayer> player = getPlayer(uid);
    if (player != nullptr && item > 0 && Item::items[item] != nullptr) {
        std::shared_ptr<ItemInstance> itemInstance =
            std::shared_ptr<ItemInstance>(new ItemInstance(item, amount, aux));
        std::shared_ptr<ItemEntity> drop = player->drop(itemInstance);
        drop->throwTime = 0;
        // logAdminAction(source, L"commands.give.success",
        // ChatPacket::e_ChatCustom, Item::items[item]->getName(itemInstance),
        // item, amount, player->getAName());
        logAdminAction(source, ChatPacket::e_ChatCustom,
                       L"commands.give.success", item, player->getAName());
    }
}

std::shared_ptr<GameCommandPacket> GiveItemCommand::preparePacket(
    std::shared_ptr<Player> player, int item, int amount, int aux,
    const std::wstring& tag) {
    if (player == nullptr) return nullptr;

    ByteArrayOutputStream baos;
    DataOutputStream dos(&baos);

    dos.writePlayerUID(player->getXuid());
    dos.writeInt(item);
    dos.writeInt(amount);
    dos.writeInt(aux);
    dos.writeUTF(tag);

    return std::shared_ptr<GameCommandPacket>(
        new GameCommandPacket(eGameCommand_Give, baos.toByteArray()));
}