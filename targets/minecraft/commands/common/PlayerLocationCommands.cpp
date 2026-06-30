#include "PlayerLocationCommands.h"

#include <memory>
#include <string>
#include <vector>

#include "java/InputOutputStream/ByteArrayInputStream.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "minecraft/Pos.h"
#include "minecraft/commands/CommandSender.h"
#include "minecraft/commands/CommandsEnum.h"
#include "minecraft/network/packet/ChatPacket.h"
#include "minecraft/server/MinecraftServer.h"
#include "minecraft/server/PlayerList.h"
#include "minecraft/server/level/ServerLevel.h"
#include "minecraft/server/level/ServerPlayer.h"
#include "minecraft/server/network/PlayerConnection.h"
#include "minecraft/world/level/dimension/Dimension.h"

namespace {

std::shared_ptr<ServerPlayer> requireServerPlayer(
    std::shared_ptr<CommandSender> source) {
    auto sp = std::dynamic_pointer_cast<ServerPlayer>(source);
    if (sp == nullptr) {
        source->sendMessage(L"§cThis command can only be used by players");
    }
    return sp;
}

// Remember the player's current location as their /back target.
void rememberBackPosition(std::shared_ptr<ServerPlayer> sp) {
    sp->m_hasBack = true;
    sp->m_backX = sp->x;
    sp->m_backY = sp->y;
    sp->m_backZ = sp->z;
    sp->m_backDim = (sp->level != nullptr && sp->level->dimension != nullptr)
                        ? sp->level->dimension->id
                        : 0;
}

void teleportTo(std::shared_ptr<ServerPlayer> sp, double x, double y, double z) {
    if (sp->connection == nullptr) return;
    sp->ride(nullptr);
    sp->connection->teleport(x, y, z, sp->yRot, sp->xRot);
}

}  // namespace

// ---- /spawn -----------------------------------------------------------------

EGameCommand SpawnCommand::getId() { return eGameCommand_Spawn; }
int SpawnCommand::getPermissionLevel() { return LEVEL_ALL; }

void SpawnCommand::execute(std::shared_ptr<CommandSender> source,
                           std::vector<uint8_t>& commandData) {
    auto sp = requireServerPlayer(source);
    if (sp == nullptr) return;

    auto* server = MinecraftServer::getInstance();
    if (server == nullptr) return;
    ServerLevel* level = server->getLevel(0);  // overworld spawn
    if (level == nullptr) {
        source->sendMessage(L"§cWorld is not loaded");
        return;
    }
    Pos* spawn = level->getSharedSpawnPos();
    if (spawn == nullptr) return;

    rememberBackPosition(sp);
    teleportTo(sp, spawn->x + 0.5, spawn->y + 1, spawn->z + 0.5);
    delete spawn;

    source->sendMessage(L"§aTeleported to spawn");
}

// ---- /sethome ---------------------------------------------------------------

EGameCommand SetHomeCommand::getId() { return eGameCommand_SetHome; }
int SetHomeCommand::getPermissionLevel() { return LEVEL_ALL; }

void SetHomeCommand::execute(std::shared_ptr<CommandSender> source,
                             std::vector<uint8_t>& commandData) {
    auto sp = requireServerPlayer(source);
    if (sp == nullptr) return;

    sp->m_hasHome = true;
    sp->m_homeX = sp->x;
    sp->m_homeY = sp->y;
    sp->m_homeZ = sp->z;
    sp->m_homeDim = (sp->level != nullptr && sp->level->dimension != nullptr)
                        ? sp->level->dimension->id
                        : 0;
    source->sendMessage(L"§aHome set");
}

// ---- /home ------------------------------------------------------------------

EGameCommand HomeCommand::getId() { return eGameCommand_Home; }
int HomeCommand::getPermissionLevel() { return LEVEL_ALL; }

void HomeCommand::execute(std::shared_ptr<CommandSender> source,
                          std::vector<uint8_t>& commandData) {
    auto sp = requireServerPlayer(source);
    if (sp == nullptr) return;

    if (!sp->m_hasHome) {
        source->sendMessage(
            L"§cYou have not set a home yet. Use /sethome first.");
        return;
    }

    int currentDim =
        (sp->level != nullptr && sp->level->dimension != nullptr)
            ? sp->level->dimension->id
            : 0;
    if (currentDim != sp->m_homeDim) {
        source->sendMessage(L"§cYour home is in a different dimension");
        return;
    }

    rememberBackPosition(sp);
    teleportTo(sp, sp->m_homeX, sp->m_homeY, sp->m_homeZ);
    source->sendMessage(L"§aTeleported home");
}

// ---- /back ------------------------------------------------------------------

EGameCommand BackCommand::getId() { return eGameCommand_Back; }
int BackCommand::getPermissionLevel() { return LEVEL_ALL; }

void BackCommand::execute(std::shared_ptr<CommandSender> source,
                          std::vector<uint8_t>& commandData) {
    auto sp = requireServerPlayer(source);
    if (sp == nullptr) return;

    if (!sp->m_hasBack) {
        source->sendMessage(L"§cNo previous location to return to.");
        return;
    }

    int currentDim =
        (sp->level != nullptr && sp->level->dimension != nullptr)
            ? sp->level->dimension->id
            : 0;
    if (currentDim != sp->m_backDim) {
        source->sendMessage(
            L"§cYour previous location is in a different dimension");
        return;
    }

    // /back swaps current and previous: the place we came from becomes the
    // new back target so /back can toggle between two spots.
    double bx = sp->m_backX;
    double by = sp->m_backY;
    double bz = sp->m_backZ;
    rememberBackPosition(sp);
    teleportTo(sp, bx, by, bz);
    source->sendMessage(L"§aTeleported to previous location");
}

// ---- /r (reply) -------------------------------------------------------------

EGameCommand ReplyCommand::getId() { return eGameCommand_Reply; }
int ReplyCommand::getPermissionLevel() { return LEVEL_ALL; }

void ReplyCommand::execute(std::shared_ptr<CommandSender> source,
                           std::vector<uint8_t>& commandData) {
    auto sp = requireServerPlayer(source);
    if (sp == nullptr) return;

    if (sp->m_lastReplyTo.empty()) {
        source->sendMessage(L"§cNobody has whispered to you yet.");
        return;
    }

    std::wstring message;
    if (!commandData.empty()) {
        try {
            ByteArrayInputStream bais(commandData);
            DataInputStream dis(&bais);
            message = dis.readUTF();
        } catch (...) {
        }
    }
    if (message.empty()) {
        source->sendMessage(L"§cUsage: /r <message>");
        return;
    }

    auto* server = MinecraftServer::getInstance();
    if (server == nullptr) return;
    auto target = server->getPlayers()->getPlayer(sp->m_lastReplyTo);
    if (target == nullptr) {
        source->sendMessage(L"§c" + sp->m_lastReplyTo + L" is no longer online");
        return;
    }

    target->sendMessage(L"§d[" + sp->getName() + L" -> me] §r" + message);
    source->sendMessage(L"§d[me -> " + target->getName() + L"] §r" + message);

    // Mutual /r: target's last reply target becomes us
    target->m_lastReplyTo = sp->getName();
}
