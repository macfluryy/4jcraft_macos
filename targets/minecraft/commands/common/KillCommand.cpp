#include "KillCommand.h"

#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "java/Class.h"
#include "java/InputOutputStream/ByteArrayInputStream.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "minecraft/commands/CommandSender.h"
#include "minecraft/commands/CommandsEnum.h"
#include "minecraft/server/MinecraftServer.h"
#include "minecraft/server/PlayerList.h"
#include "minecraft/server/level/ServerLevel.h"
#include "minecraft/server/level/ServerPlayer.h"
#include "minecraft/world/damageSource/DamageSource.h"
#include "minecraft/world/entity/Entity.h"
#include "minecraft/world/entity/LivingEntity.h"
#include "minecraft/world/entity/player/Player.h"
#include "minecraft/world/level/Level.h"

EGameCommand KillCommand::getId() { return eGameCommand_Kill; }

int KillCommand::getPermissionLevel() { return LEVEL_ALL; }

// Kill targets:
//  TARGET_SELF      - kill source player
//  TARGET_PLAYER    - kill specific player by name
//  TARGET_MOBS      - kill all hostile mobs
//  TARGET_ANIMALS   - kill all animals
//  TARGET_ALL_ENTS  - kill all entities (excluding players)
//  TARGET_ALL_PLAYERS - kill all players
static const int KILL_TARGET_SELF = 0;
static const int KILL_TARGET_PLAYER = 1;
static const int KILL_TARGET_MOBS = 2;
static const int KILL_TARGET_ANIMALS = 3;
static const int KILL_TARGET_ALL_ENTS = 4;
static const int KILL_TARGET_ALL_PLAYERS = 5;

void KillCommand::execute(std::shared_ptr<CommandSender> source,
                          std::vector<uint8_t>& commandData) {
    int killType = KILL_TARGET_SELF;
    std::wstring targetName;

    if (!commandData.empty()) {
        try {
            ByteArrayInputStream bais(commandData);
            DataInputStream dis(&bais);
            killType = dis.readInt();
            if (killType == KILL_TARGET_PLAYER) {
                targetName = dis.readUTF();
            }
        } catch (const std::exception&) {
            killType = KILL_TARGET_SELF;
        }
    }

    MinecraftServer* server = MinecraftServer::getInstance();
    if (server == nullptr) return;

    auto sourcePlayer = std::dynamic_pointer_cast<ServerPlayer>(source);

    switch (killType) {
        case KILL_TARGET_SELF: {
            if (sourcePlayer != nullptr) {
                sourcePlayer->hurt(DamageSource::outOfWorld,
                                   std::numeric_limits<float>::max());
                source->sendMessage(L"§aOuch. That look like it hurt.");
            }
            break;
        }
        case KILL_TARGET_PLAYER: {
            auto target = server->getPlayers()->getPlayer(targetName);
            if (target == nullptr) {
                source->sendMessage(L"§cPlayer not found: " + targetName);
                return;
            }
            target->hurt(DamageSource::outOfWorld,
                         std::numeric_limits<float>::max());
            source->sendMessage(L"§aKilled " + targetName);
            break;
        }
        case KILL_TARGET_MOBS:
        case KILL_TARGET_ANIMALS:
        case KILL_TARGET_ALL_ENTS: {
            int killed = 0;
            for (size_t i = 0; i < server->levels.size(); i++) {
                ServerLevel* level = server->levels[i];
                if (level == nullptr) continue;
                auto allEntities = ((Level*)level)->getAllEntities();
                for (auto& e : allEntities) {
                    if (e == nullptr) continue;
                    // Never kill players via these targets
                    if (e->instanceof(eTYPE_PLAYER)) continue;

                    bool shouldKill = false;
                    if (killType == KILL_TARGET_MOBS) {
                        shouldKill = e->instanceof(eTYPE_MONSTER);
                    } else if (killType == KILL_TARGET_ANIMALS) {
                        // Animals + ambient + water animals etc.
                        shouldKill = e->instanceof(eTYPE_MOB) &&
                                     !e->instanceof(eTYPE_MONSTER);
                    } else {
                        // ALL_ENTS - any entity except players
                        shouldKill = true;
                    }

                    if (shouldKill && e->isAlive()) {
                        if (e->instanceof(eTYPE_LIVINGENTITY)) {
                            std::dynamic_pointer_cast<LivingEntity>(e)->hurt(
                                DamageSource::outOfWorld,
                                std::numeric_limits<float>::max());
                        } else {
                            e->remove();
                        }
                        killed++;
                    }
                }
            }
            source->sendMessage(L"§aKilled " + std::to_wstring(killed) +
                                L" entities");
            break;
        }
        case KILL_TARGET_ALL_PLAYERS: {
            int killed = 0;
            for (auto& p : server->getPlayers()->players) {
                if (p == nullptr) continue;
                p->hurt(DamageSource::outOfWorld,
                        std::numeric_limits<float>::max());
                killed++;
            }
            source->sendMessage(L"§aKilled " + std::to_wstring(killed) +
                                L" players");
            break;
        }
        default:
            source->sendMessage(L"§cUnknown kill target");
            break;
    }
}
