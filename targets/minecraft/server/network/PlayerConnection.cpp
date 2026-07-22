#include "PlayerConnection.h"

#include <wchar.h>
#include <sstream>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <format>
#include <utility>

#include "app/common/App_enums.h"
#include "app/common/src/Console_Debug_enum.h"
#include "app/common/src/DLC/DLCManager.h"
#include "app/common/src/DLC/DLCSkinFile.h"
#include "app/common/src/Network/GameNetworkManager.h"
#include "app/common/src/Network/NetworkPlayerInterface.h"
#include "app/common/src/Network/Socket.h"
#include "app/mac/MacGame.h"
#include "app/include/SkinBox.h"
#include "ServerConnection.h"
#include "java/Class.h"
#include "java/InputOutputStream/ByteArrayInputStream.h"
#include "java/InputOutputStream/ByteArrayOutputStream.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "java/InputOutputStream/DataOutputStream.h"
#include "java/JavaMath.h"
#include "java/Random.h"
#include "java/System.h"
#include "minecraft/Facing.h"
#include "minecraft/SharedConstants.h"
#include "minecraft/commands/CommandDispatcher.h"
#include "minecraft/commands/CommandsEnum.h"
#include "minecraft/network/Connection.h"
#include "minecraft/network/packet/AnimatePacket.h"
#include "minecraft/network/packet/ChatPacket.h"
#include "minecraft/network/packet/ClientCommandPacket.h"
#include "minecraft/network/packet/ClientInformationPacket.h"
#include "minecraft/network/packet/ContainerAckPacket.h"
#include "minecraft/network/packet/ContainerButtonClickPacket.h"
#include "minecraft/network/packet/ContainerClickPacket.h"
#include "minecraft/network/packet/ContainerSetSlotPacket.h"
#include "minecraft/network/packet/CraftItemPacket.h"
#include "minecraft/network/packet/CustomPayloadPacket.h"
#include "minecraft/network/packet/DebugOptionsPacket.h"
#include "minecraft/network/packet/DisconnectPacket.h"
#include "minecraft/network/packet/GameCommandPacket.h"
#include "minecraft/network/packet/GameEventPacket.h"
#include "minecraft/network/packet/InteractPacket.h"
#include "minecraft/network/packet/KeepAlivePacket.h"
#include "minecraft/network/packet/KickPlayerPacket.h"
#include "minecraft/network/packet/MovePlayerPacket.h"
#include "minecraft/network/packet/Packet.h"
#include "minecraft/network/packet/PlayerAbilitiesPacket.h"
#include "minecraft/network/packet/PlayerActionPacket.h"
#include "minecraft/network/packet/PlayerCommandPacket.h"
#include "minecraft/network/packet/PlayerInfoPacket.h"
#include "minecraft/network/packet/PlayerInputPacket.h"
#include "minecraft/network/packet/ServerSettingsChangedPacket.h"
#include "minecraft/network/packet/SetCarriedItemPacket.h"
#include "minecraft/network/packet/SetCreativeModeSlotPacket.h"
#include "minecraft/network/packet/SignUpdatePacket.h"
#include "minecraft/network/packet/TextureAndGeometryChangePacket.h"
#include "minecraft/network/packet/TextureAndGeometryPacket.h"
#include "minecraft/network/packet/TextureChangePacket.h"
#include "minecraft/network/packet/TexturePacket.h"
#include "minecraft/network/packet/TileUpdatePacket.h"
#include "minecraft/network/packet/TradeItemPacket.h"
#include "minecraft/network/packet/UseItemPacket.h"
#include "minecraft/server/MinecraftServer.h"
#include "minecraft/server/PlayerList.h"
#include "minecraft/server/level/ServerLevel.h"
#include "minecraft/server/level/ServerPlayer.h"
#include "minecraft/server/level/ServerPlayerGameMode.h"
#include "minecraft/stats/GenericStats.h"
#include "minecraft/world/entity/Entity.h"
#include "minecraft/world/entity/animal/EntityHorse.h"
#include "minecraft/world/entity/item/ItemEntity.h"
#include "minecraft/world/entity/player/Abilities.h"
#include "minecraft/world/entity/player/Inventory.h"
#include "minecraft/world/entity/player/Player.h"
#include "minecraft/world/food/FoodConstants.h"
#include "minecraft/world/inventory/AbstractContainerMenu.h"
#include "minecraft/world/inventory/AnvilMenu.h"
#include "minecraft/world/inventory/BeaconMenu.h"
#include "minecraft/world/inventory/CraftingMenu.h"
#include "minecraft/world/inventory/InventoryMenu.h"
#include "minecraft/world/inventory/MerchantMenu.h"
#include "minecraft/world/inventory/Slot.h"
#include "minecraft/world/item/Item.h"
#include "minecraft/world/item/ItemInstance.h"
#include "minecraft/world/item/MapItem.h"
#include "minecraft/world/item/crafting/Recipes.h"
#include "minecraft/world/item/crafting/Recipy.h"
#include "minecraft/world/item/trading/Merchant.h"
#include "minecraft/world/item/trading/MerchantRecipe.h"
#include "minecraft/world/item/trading/MerchantRecipeList.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/LevelSettings.h"
#include "minecraft/world/level/ViewDistanceUtil.h"
#include "minecraft/world/level/dimension/Dimension.h"
#include "minecraft/world/level/saveddata/MapItemSavedData.h"
#include "minecraft/world/level/tile/Tile.h"
#include "minecraft/world/level/tile/entity/BeaconTileEntity.h"
#include "minecraft/world/level/tile/entity/CommandBlockEntity.h"
#include "minecraft/world/level/tile/entity/SignTileEntity.h"
#include "minecraft/world/level/tile/entity/TileEntity.h"
#include "minecraft/world/phys/AABB.h"

class SavedData;

Random PlayerConnection::random;

PlayerConnection::PlayerConnection(MinecraftServer* server,
                                   Connection* connection,
                                   std::shared_ptr<ServerPlayer> player) {
    
    done = false;
    tickCount = 0;
    aboveGroundTickCount = 0;
    xLastOk = yLastOk = zLastOk = 0;
    synched = true;
    didTick = false;
    lastKeepAliveId = 0;
    lastKeepAliveTime = 0;
    lastKeepAliveTick = 0;
    chatSpamTickCount = 0;
    dropSpamTickCount = 0;

    this->server = server;
    this->connection = connection;
    connection->setListener(this);
    this->player = player;
    
    
    m_bCloseOnTick = false;
    m_bWasKicked = false;

    m_friendsOnlyUGC = false;
    m_offlineXUID = INVALID_XUID;
    m_onlineXUID = INVALID_XUID;
    m_bHasClientTickedOnce = false;

    setShowOnMaps(
        app.GetGameHostOption(eGameHostOption_Gamertags) != 0 ? true : false);
}

PlayerConnection::~PlayerConnection() { delete connection; }

void PlayerConnection::tick() {
    if (done) return;

    if (m_bCloseOnTick) {
        disconnect(DisconnectPacket::eDisconnect_Closed);
        return;
    }

    didTick = false;
    tickCount++;
    connection->tick();
    if (done) return;

    
    
    
    
    
    int pendingVD =
        m_pendingClientViewDistance.exchange(-1, std::memory_order_relaxed);
    if (pendingVD >= 0 && player != nullptr) {
        
        
        int serverVD = server->getPlayers()->getViewDistance();
        int effective = pendingVD < serverVD ? pendingVD : serverVD;
        player->setEffectiveViewDistance(effective);
    }

    if ((tickCount - lastKeepAliveTick) > 20 * 1) {
        lastKeepAliveTick = tickCount;
        lastKeepAliveTime = System::nanoTime() / 1000000;
        lastKeepAliveId = random.nextInt();
        send(std::shared_ptr<KeepAlivePacket>(
            new KeepAlivePacket(lastKeepAliveId)));
    }

    if (chatSpamTickCount > 0) {
        chatSpamTickCount--;
    }
    if (dropSpamTickCount > 0) {
        dropSpamTickCount--;
    }
}

void PlayerConnection::disconnect(DisconnectPacket::eDisconnectReason reason) {
    std::lock_guard<std::mutex> lock(done_cs);
    if (done) {
        return;
    }

    app.DebugPrintf("PlayerConnection disconect reason: %d\n", reason);
    player->disconnect();

    
    
    server->getPlayers()->removePlayerFromReceiving(player);
    send(std::make_shared<DisconnectPacket>(reason));
    connection->sendAndQuit();
    
    
    
    
    if (getWasKicked()) {
        server->getPlayers()->broadcastAll(std::make_shared<ChatPacket>(
            player->name, ChatPacket::e_ChatPlayerKickedFromGame));
    } else {
        server->getPlayers()->broadcastAll(std::shared_ptr<ChatPacket>(
            new ChatPacket(player->name, ChatPacket::e_ChatPlayerLeftGame)));
    }

    server->getPlayers()->remove(player);
    done = true;
}

void PlayerConnection::handlePlayerInput(
    std::shared_ptr<PlayerInputPacket> packet) {
    player->setPlayerInput(packet->getXxa(), packet->getYya(),
                           packet->isJumping(), packet->isSneaking());
}

void PlayerConnection::handleMovePlayer(
    std::shared_ptr<MovePlayerPacket> packet) {
    ServerLevel* level = server->getLevel(player->dimension);

    didTick = true;
    if (synched) m_bHasClientTickedOnce = true;

    if (player->wonGame) return;

    if (!synched) {
        double yDiff = packet->y - yLastOk;
        if (packet->x == xLastOk && yDiff * yDiff < 0.01 &&
            packet->z == zLastOk) {
            synched = true;
        }
    }

    if (synched) {
        if (player->riding != nullptr) {
            float yRotT = player->yRot;
            float xRotT = player->xRot;
            player->riding->positionRider();
            double xt = player->x;
            double yt = player->y;
            double zt = player->z;

            if (packet->hasRot) {
                yRotT = packet->yRot;
                xRotT = packet->xRot;
            }

            player->onGround = packet->onGround;

            player->doTick(false);
            player->ySlideOffset = 0;
            player->absMoveTo(xt, yt, zt, yRotT, xRotT);
            if (player->riding != nullptr) player->riding->positionRider();
            server->getPlayers()->move(player);

            
            
            if (synched) {
                xLastOk = player->x;
                yLastOk = player->y;
                zLastOk = player->z;
            }
            ((Level*)level)->tick(player);

            return;
        }

        if (player->isSleeping()) {
            player->doTick(false);
            player->absMoveTo(xLastOk, yLastOk, zLastOk, player->yRot,
                              player->xRot);
            ((Level*)level)->tick(player);
            return;
        }

        double startY = player->y;
        xLastOk = player->x;
        yLastOk = player->y;
        zLastOk = player->z;

        double xt = player->x;
        double yt = player->y;
        double zt = player->z;

        float yRotT = player->yRot;
        float xRotT = player->xRot;

        if (packet->hasPos && packet->y == -999 && packet->yView == -999) {
            packet->hasPos = false;
        }

        if (packet->hasPos) {
            xt = packet->x;
            yt = packet->y;
            zt = packet->z;
            double yd = packet->yView - packet->y;
            if (!player->isSleeping() && (yd > 1.65 || yd < 0.1)) {
                disconnect(DisconnectPacket::eDisconnect_IllegalStance);
                
                
                return;
            }
            if (std::abs(packet->x) > 32000000 ||
                std::abs(packet->z) > 32000000) {
                disconnect(DisconnectPacket::eDisconnect_IllegalPosition);
                return;
            }
        }
        if (packet->hasRot) {
            yRotT = packet->yRot;
            xRotT = packet->xRot;
        }

        
        
        if (player->abilities.mayfly || player->isAllowedToFly()) {
            player->abilities.flying = packet->isFlying;
        } else
            player->abilities.flying = false;

        player->doTick(false);
        player->ySlideOffset = 0;
        player->absMoveTo(xLastOk, yLastOk, zLastOk, yRotT, xRotT);

        if (!synched) return;

        double xDist = xt - player->x;
        double yDist = yt - player->y;
        double zDist = zt - player->z;

        double dist = xDist * xDist + yDist * yDist + zDist * zDist;

        
        










        float r = 1 / 16.0f;
        AABB shrunk = player->bb.shrink(r, r, r);
        bool oldOk = level->getCubes(player, &shrunk)->empty();

        if (player->onGround && !packet->onGround && yDist > 0) {
            
            player->causeFoodExhaustion(FoodConstants::EXHAUSTION_JUMP);
        }

        player->move(xDist, yDist, zDist);

        
        
        
        
        
        
        if (!synched) return;

        player->onGround = packet->onGround;
        
        
        player->checkMovementStatistiscs(xDist, yDist, zDist);

        double oyDist = yDist;

        xDist = xt - player->x;
        yDist = yt - player->y;

        
        if (yDist > -0.5 || yDist < 0.5) {
            yDist = 0;
        }
        zDist = zt - player->z;
        dist = xDist * xDist + yDist * yDist + zDist * zDist;
        bool fail = false;
        if (dist > 0.25 * 0.25 && !player->isSleeping() &&
            !player->gameMode->isCreative() && !player->isAllowedToFly()) {
            fail = true;
            
            
            
            
#if !defined(_CONTENT_PACKAGE)
            wprintf(L"%ls moved wrongly!\n", player->name.c_str());
            app.DebugPrintf("Got position %f, %f, %f\n", xt, yt, zt);
            app.DebugPrintf("Expected %f, %f, %f\n", player->x, player->y,
                            player->z);
#endif
        }
        player->absMoveTo(xt, yt, zt, yRotT, xRotT);

        
        shrunk = player->bb.shrink(r, r, r);
        bool newOk = level->getCubes(player, &shrunk)->empty();
        if (oldOk && (fail || !newOk) && !player->isSleeping()) {
            teleport(xLastOk, yLastOk, zLastOk, yRotT, xRotT);
            return;
        }
        AABB testBox = player->bb.grow(r, r, r).expand(0, -0.55, 0);
        
        if (!server->isFlightAllowed() && !player->gameMode->isCreative() &&
            !level->containsAnyBlocks(&testBox) && !player->isAllowedToFly()) {
            if (oyDist >= (-0.5f / 16.0f)) {
                aboveGroundTickCount++;
                if (aboveGroundTickCount > 80) {
                    
                    
#if !defined(_CONTENT_PACKAGE)
                    wprintf(L"%ls was kicked for floating too long!\n",
                            player->name.c_str());
#endif
                    disconnect(DisconnectPacket::eDisconnect_NoFlying);
                    return;
                }
            }
        } else {
            aboveGroundTickCount = 0;
        }

        player->onGround = packet->onGround;
        server->getPlayers()->move(player);
        player->doCheckFallDamage(player->y - startY, packet->onGround);
    } else if ((tickCount % SharedConstants::TICKS_PER_SECOND) == 0) {
        teleport(xLastOk, yLastOk, zLastOk, player->yRot, player->xRot);
    }
}

void PlayerConnection::teleport(double x, double y, double z, float yRot,
                                float xRot, bool sendPacket ) {
    synched = false;
    xLastOk = x;
    yLastOk = y;
    zLastOk = z;
    player->absMoveTo(x, y, z, yRot, xRot);
    
    
    
    
    if (sendPacket)
        player->connection->send(std::make_shared<MovePlayerPacket::PosRot>(
            x, y + 1.62f, y, z, yRot, xRot, false, false));
}

void PlayerConnection::handlePlayerAction(
    std::shared_ptr<PlayerActionPacket> packet) {
    ServerLevel* level = server->getLevel(player->dimension);
    player->resetLastActionTime();

    if (packet->action == PlayerActionPacket::DROP_ITEM) {
        player->drop(false);
        return;
    } else if (packet->action == PlayerActionPacket::DROP_ALL_ITEMS) {
        player->drop(true);
        return;
    } else if (packet->action == PlayerActionPacket::RELEASE_USE_ITEM) {
        player->releaseUsingItem();
        return;
    }

    bool shouldVerifyLocation = false;
    if (packet->action == PlayerActionPacket::START_DESTROY_BLOCK)
        shouldVerifyLocation = true;
    if (packet->action == PlayerActionPacket::ABORT_DESTROY_BLOCK)
        shouldVerifyLocation = true;
    if (packet->action == PlayerActionPacket::STOP_DESTROY_BLOCK)
        shouldVerifyLocation = true;

    int x = packet->x;
    int y = packet->y;
    int z = packet->z;
    if (shouldVerifyLocation) {
        double xDist = player->x - (x + 0.5);
        
        
        double yDist = player->y - (y + 0.5) + 1.5;
        double zDist = player->z - (z + 0.5);
        double dist = xDist * xDist + yDist * yDist + zDist * zDist;
        if (dist > 6 * 6) {
            return;
        }
        if (y >= server->getMaxBuildHeight()) {
            return;
        }
    }

    if (packet->action == PlayerActionPacket::START_DESTROY_BLOCK) {
        if (true)
            player->gameMode->startDestroyBlock(
                x, y, z,
                packet->face);  
                                
                                
                                
        else
            player->connection->send(std::shared_ptr<TileUpdatePacket>(
                new TileUpdatePacket(x, y, z, level)));

    } else if (packet->action == PlayerActionPacket::STOP_DESTROY_BLOCK) {
        player->gameMode->stopDestroyBlock(x, y, z);
        server->getPlayers()->prioritiseTileChanges(
            x, y, z,
            level->dimension
                ->id);  
                        
        if (level->getTile(x, y, z) != 0)
            player->connection->send(std::shared_ptr<TileUpdatePacket>(
                new TileUpdatePacket(x, y, z, level)));
    } else if (packet->action == PlayerActionPacket::ABORT_DESTROY_BLOCK) {
        player->gameMode->abortDestroyBlock(x, y, z);
        if (level->getTile(x, y, z) != 0)
            player->connection->send(std::shared_ptr<TileUpdatePacket>(
                new TileUpdatePacket(x, y, z, level)));
    }
}

void PlayerConnection::handleUseItem(std::shared_ptr<UseItemPacket> packet) {
    ServerLevel* level = server->getLevel(player->dimension);
    std::shared_ptr<ItemInstance> item = player->inventory->getSelected();
    bool informClient = false;
    int x = packet->getX();
    int y = packet->getY();
    int z = packet->getZ();
    int face = packet->getFace();
    player->resetLastActionTime();

    
    bool canEditSpawn =
        level->canEditSpawn;  
                              
    if (packet->getFace() == 255) {
        if (item == nullptr) return;
        player->gameMode->useItem(player, level, item);
    } else if ((packet->getY() < server->getMaxBuildHeight() - 1) ||
               (packet->getFace() != Facing::UP &&
                packet->getY() < server->getMaxBuildHeight())) {
        if (synched &&
            player->distanceToSqr(x + 0.5, y + 0.5, z + 0.5) < 8 * 8) {
            if (true)  
                       
                       
                       
            {
                player->gameMode->useItemOn(
                    player, level, item, x, y, z, face, packet->getClickX(),
                    packet->getClickY(), packet->getClickZ());
            }
        }

        informClient = true;
    } else {
        
        
        
        informClient = true;
    }

    if (informClient) {
        player->connection->send(std::shared_ptr<TileUpdatePacket>(
            new TileUpdatePacket(x, y, z, level)));

        if (face == 0) y--;
        if (face == 1) y++;
        if (face == 2) z--;
        if (face == 3) z++;
        if (face == 4) x--;
        if (face == 5) x++;

        
        
        
        
        
        
        
        
        
        
        
        if (level->getTile(x, y, z) != Tile::pistonMovingPiece_Id) {
            player->connection->send(std::shared_ptr<TileUpdatePacket>(
                new TileUpdatePacket(x, y, z, level)));
        }
    }

    item = player->inventory->getSelected();

    bool forceClientUpdate = false;
    if (item != nullptr && packet->getItem() == nullptr) {
        forceClientUpdate = true;
    }
    if (item != nullptr && item->count == 0) {
        player->inventory->items[player->inventory->selected] = nullptr;
        item = nullptr;
    }

    if (item == nullptr || item->getUseDuration() == 0) {
        player->ignoreSlotUpdateHack = true;
        player->inventory->items[player->inventory->selected] =
            ItemInstance::clone(
                player->inventory->items[player->inventory->selected]);
        Slot* s = player->containerMenu->getSlotFor(
            player->inventory, player->inventory->selected);
        player->containerMenu->broadcastChanges();
        player->ignoreSlotUpdateHack = false;

        if (forceClientUpdate ||
            !ItemInstance::matches(player->inventory->getSelected(),
                                   packet->getItem())) {
            send(std::shared_ptr<ContainerSetSlotPacket>(
                new ContainerSetSlotPacket(player->containerMenu->containerId,
                                           s->index,
                                           player->inventory->getSelected())));
        }
    }
}

void PlayerConnection::onDisconnect(DisconnectPacket::eDisconnectReason reason,
                                    void* reasonObjects) {
    std::lock_guard<std::mutex> lock(done_cs);
    if (done) return;
    
    
    
    
    
    if (getWasKicked()) {
        server->getPlayers()->broadcastAll(std::make_shared<ChatPacket>(
            player->name, ChatPacket::e_ChatPlayerKickedFromGame));
    } else {
        server->getPlayers()->broadcastAll(std::shared_ptr<ChatPacket>(
            new ChatPacket(player->name, ChatPacket::e_ChatPlayerLeftGame)));
    }
    server->getPlayers()->remove(player);
    done = true;
}

void PlayerConnection::onUnhandledPacket(std::shared_ptr<Packet> packet) {
    
    
    disconnect(DisconnectPacket::eDisconnect_UnexpectedPacket);
}

void PlayerConnection::send(std::shared_ptr<Packet> packet) {
    if (connection->getSocket() != nullptr) {
        if (!server->getPlayers()->canReceiveAllPackets(player)) {
            
            if (!Packet::canSendToAnyClient(packet)) {
                
                
                
                return;
            }
        }
        connection->send(packet);
    }
}


void PlayerConnection::queueSend(std::shared_ptr<Packet> packet) {
    if (connection->getSocket() != nullptr) {
        if (!server->getPlayers()->canReceiveAllPackets(player)) {
            
            if (!Packet::canSendToAnyClient(packet)) {
                
                
                
                return;
            }
        }
        connection->queueSend(packet);
    }
}

void PlayerConnection::handleSetCarriedItem(
    std::shared_ptr<SetCarriedItemPacket> packet) {
    if (packet->slot < 0 || packet->slot >= Inventory::getSelectionSize()) {
        
        
        return;
    }
    player->inventory->selected = packet->slot;
    player->resetLastActionTime();
}

void PlayerConnection::handleChat(std::shared_ptr<ChatPacket> packet) {
    if (packet->m_stringArgs.empty()) return;
    std::wstring message = packet->m_stringArgs[0];

    
    
    
    if (message.size() > 256) {
        message.resize(256);
    }

    
    
    
    if (!message.empty() && message[0] != L'/') {
        chatSpamTickCount += 20;
        if (chatSpamTickCount > 200) {
            disconnect(DisconnectPacket::eDisconnect_Kicked);
            return;
        }
    }

    if (!message.empty() && message[0] == L'/') {
        handleCommand(message);
    } else if (!message.empty()) {
        
        
        
        
        
        auto chatPacket = std::make_shared<ChatPacket>(
            player->getName(), ChatPacket::e_ChatCustom, -1);
        chatPacket->m_stringArgs.push_back(message);
        server->getPlayers()->broadcastAll(chatPacket);
    }
}


static std::wstring toLower(const std::wstring& str) {
    std::wstring result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](wchar_t c) { return std::tolower(static_cast<unsigned char>(c)); });
    return result;
}

void PlayerConnection::handleCommand(const std::wstring& message) {
    
    
    std::wstringstream ss(message.substr(1));
    std::wstring cmdName;
    ss >> cmdName;
    cmdName = toLower(cmdName);  
    
    
    EGameCommand cmd = parseCommandName(cmdName);
    
    
    if (cmd == eGameCommand_COUNT) {  
        player->sendMessage(L"Unknown command: " + cmdName);
        return;
    }
    
    
    
    std::vector<uint8_t> commandData;
    std::wstring arg1, arg2, arg3;
    std::wstring arg1_orig, arg2_orig, arg3_orig;  
    ss >> arg1;  
    ss >> arg2;  
    ss >> arg3;  
    
    
    arg1_orig = arg1;
    arg2_orig = arg2;
    arg3_orig = arg3;
    
    
    arg1 = toLower(arg1);
    arg2 = toLower(arg2);
    arg3 = toLower(arg3);
    
    
    
    try {
        ByteArrayOutputStream baos;
        DataOutputStream dos(&baos);
        
        switch (cmd) {
            case eGameCommand_Time: {
                
                if (arg1 == L"set") {
                    int timeValue = 0;
                    
                    if (arg2 == L"sunrise") {
                        timeValue = 0;       
                    } else if (arg2 == L"day") {
                        timeValue = 1000;    
                    } else if (arg2 == L"noon") {
                        timeValue = 6000;    
                    } else if (arg2 == L"sunset") {
                        timeValue = 12000;   
                    } else if (arg2 == L"night") {
                        timeValue = 13000;   
                    } else if (arg2 == L"midnight") {
                        timeValue = 18000;   
                    } else {
                        
                        try {
                            timeValue = std::stoi(arg2);
                            
                            if (timeValue < 0 || timeValue > 23999) {
                                player->sendMessage(L"§cTime must be between 0 and 23999");
                                return;
                            }
                        } catch (...) {
                            player->sendMessage(L"§cInvalid time value: " + arg2);
                            player->sendMessage(L"§cUsage: /time set [sunrise|day|noon|sunset|night|midnight|N]");
                            return;
                        }
                    }
                    dos.writeInt(timeValue);
                } else {
                    player->sendMessage(L"§cUsage: /time set [sunrise|day|noon|sunset|night|midnight|N]");
                    return;
                }
                break;
            }
            
            case eGameCommand_GameMode: {
                
                int gameModeId = -1;  
                
                if (arg1 == L"survival" || arg1 == L"s") {
                    gameModeId = 0;
                } else if (arg1 == L"creative" || arg1 == L"c") {
                    gameModeId = 1;
                } else if (arg1 == L"adventure" || arg1 == L"a") {
                    gameModeId = 2;
                } else if (arg1 == L"spectator" || arg1 == L"sp" || arg1 == L"spc") {
                    gameModeId = 3;
                } else {
                    
                    try {
                        gameModeId = std::stoi(arg1);
                        if (gameModeId < 0 || gameModeId > 3) {
                            player->sendMessage(L"§cInvalid gamemode ID: " + arg1);
                            player->sendMessage(L"§cUsage: /gamemode [survival|creative|adventure|spectator]");
                            return;
                        }
                    } catch (...) {
                        player->sendMessage(L"§cUnknown game mode: " + arg1);
                        player->sendMessage(L"§cUsage: /gamemode [survival|creative|adventure|spectator]");
                        return;
                    }
                }
                
                dos.writeInt(gameModeId);
                
                if (!arg2_orig.empty()) {
                    
                    dos.writeUTF(arg2_orig);
                }
                break;
            }
            
            case eGameCommand_Give: {
                
                if (arg1_orig.empty() || arg2_orig.empty()) {
                    player->sendMessage(L"Usage: /give <player> <item> [count] [data]");
                    return;
                }
                dos.writeUTF(arg1_orig);  
                dos.writeUTF(arg2_orig);  
                int count = 1;
                if (!arg3.empty()) {
                    try {
                        count = std::stoi(arg3);
                    } catch (...) {
                        count = 1;
                    }
                }
                dos.writeInt(count);
                break;
            }
            
            case eGameCommand_Kill: {
                
                int killType = 0;  
                std::wstring targetPlayer;
                if (arg1.empty() || arg1 == L"@s" || arg1 == L"self" ||
                    arg1 == L"me") {
                    killType = 0;  
                } else if (arg1 == L"@e" || arg1 == L"all" ||
                           arg1 == L"entities") {
                    killType = 4;  
                } else if (arg1 == L"@a") {
                    killType = 5;  
                } else if (arg1 == L"mobs" || arg1 == L"monsters" ||
                           arg1 == L"hostile") {
                    killType = 2;  
                } else if (arg1 == L"animals" || arg1 == L"passive") {
                    killType = 3;  
                } else {
                    
                    killType = 1;  
                    targetPlayer = arg1_orig;
                }
                dos.writeInt(killType);
                if (killType == 1) {
                    dos.writeUTF(targetPlayer);
                }
                break;
            }
            
            case eGameCommand_Teleport: {
                
                
                
                
                if (arg1_orig.empty()) {
                    player->sendMessage(L"§cUsage: /tp <player> | <x> <y> <z> | <subject> <target> | <subject> <x> <y> <z>");
                    return;
                }

                auto isNumeric = [](const std::wstring& s) {
                    if (s.empty()) return false;
                    try {
                        std::stod(s);
                        return true;
                    } catch (...) {
                        return false;
                    }
                };

                if (arg2.empty()) {
                    
                    dos.writeInt(0);  
                    dos.writeUTF(arg1_orig);
                } else if (arg3.empty()) {
                    
                    dos.writeInt(2);  
                    dos.writeUTF(arg1_orig);
                    dos.writeUTF(arg2_orig);
                } else {
                    
                    std::wstring extra4;
                    ss >> extra4;
                    if (isNumeric(arg1) && isNumeric(arg2) && isNumeric(arg3)) {
                        
                        dos.writeInt(1);  
                        dos.writeDouble(std::stod(arg1));
                        dos.writeDouble(std::stod(arg2));
                        dos.writeDouble(std::stod(arg3));
                    } else if (!extra4.empty() && isNumeric(arg2) &&
                               isNumeric(arg3) && isNumeric(extra4)) {
                        
                        dos.writeInt(3);  
                        dos.writeUTF(arg1_orig);
                        dos.writeDouble(std::stod(arg2));
                        dos.writeDouble(std::stod(arg3));
                        dos.writeDouble(std::stod(extra4));
                    } else {
                        player->sendMessage(L"§cInvalid /tp arguments");
                        return;
                    }
                }
                break;
            }
            
            case eGameCommand_ToggleDownfall: {
                
                int weatherType = 0;
                if (arg1 == L"clear") {
                    weatherType = 0;
                } else if (arg1 == L"rain") {
                    weatherType = 1;
                } else if (arg1 == L"thunder" || arg1 == L"thunderstorm") {
                    weatherType = 2;
                } else {
                    player->sendMessage(L"§cUnknown weather type: " + arg1);
                    player->sendMessage(L"§cUsage: /weather [clear|rain|thunder]");
                    return;
                }
                dos.writeInt(weatherType);
                break;
            }
            
            case eGameCommand_Effect: {
                
                if (arg1_orig.empty() || arg2_orig.empty()) {
                    player->sendMessage(L"Usage: /effect <player> <effect> [duration] [amplifier]");
                    return;
                }
                dos.writeUTF(arg1_orig);  
                dos.writeUTF(arg2_orig);  
                
                break;
            }
            
            case eGameCommand_Experience: {
                
                if (arg1.empty()) {
                    player->sendMessage(L"§cUsage: /xp <amount>[L|l] [player]");
                    return;
                }
                std::wstring amountStr = arg1;
                bool levels = false;
                if (!amountStr.empty()) {
                    wchar_t last = amountStr.back();
                    if (last == L'l' || last == L'L') {
                        levels = true;
                        amountStr.pop_back();
                    }
                }
                int amount = 0;
                try {
                    amount = std::stoi(amountStr);
                } catch (...) {
                    player->sendMessage(L"§cInvalid xp amount: " + amountStr);
                    return;
                }
                dos.writeInt(amount);
                dos.writeBoolean(levels);
                dos.writeUTF(arg2_orig);  
                break;
            }

            case eGameCommand_Summon: {
                
                if (arg1_orig.empty()) {
                    player->sendMessage(L"§cUsage: /summon <mob> [x] [y] [z]");
                    return;
                }
                dos.writeUTF(arg1_orig);

                bool hasCoords = !arg2.empty() && !arg3.empty();
                std::wstring extra4;
                if (hasCoords) ss >> extra4;
                hasCoords = hasCoords && !extra4.empty();

                dos.writeBoolean(hasCoords);
                if (hasCoords) {
                    try {
                        double x = std::stod(arg2);
                        double y = std::stod(arg3);
                        double z = std::stod(extra4);
                        dos.writeDouble(x);
                        dos.writeDouble(y);
                        dos.writeDouble(z);
                    } catch (...) {
                        player->sendMessage(L"§cInvalid coordinates");
                        return;
                    }
                }
                break;
            }

            case eGameCommand_Help: {
                
                player->sendMessage(L"§e--- Available commands ---");
                player->sendMessage(L"§e/gamemode <s|c|a|sp>  §7- change gamemode");
                player->sendMessage(L"§e/give <player> <item> [count]  §7- give items");
                player->sendMessage(L"§e/time set <value>  §7- set world time");
                player->sendMessage(L"§e/weather <clear|rain|thunder>  §7- set weather");
                player->sendMessage(L"§e/kill [self|@e|@a|mobs|animals|<player>]");
                player->sendMessage(L"§e/tp <player> | <x> <y> <z>");
                player->sendMessage(L"§e/xp <amount>[L] [player]  §7- give xp");
                player->sendMessage(L"§e/summon <mob> [x] [y] [z]");
                player->sendMessage(L"§e/effect <player> <effect>");
                player->sendMessage(L"§e/enchant <enchantmentId> <level>");
                player->sendMessage(L"§e/say <message>  §7- broadcast a message");
                player->sendMessage(L"§e--- Multiplayer ---");
                player->sendMessage(L"§e/list  §7- list online players");
                player->sendMessage(L"§e/msg <player> <message>  §7- private message");
                player->sendMessage(L"§e/kick <player> [reason]  §7- kick a player");
                player->sendMessage(L"§e/heal [player]  §7- restore health");
                player->sendMessage(L"§e/feed [player]  §7- restore hunger");
                player->sendMessage(L"§e/seed  §7- show world seed");
                player->sendMessage(L"§e--- Admin ---");
                player->sendMessage(L"§e/op <player>  §7- grant operator");
                player->sendMessage(L"§e/deop <player>  §7- revoke operator");
                player->sendMessage(L"§e/ban <player>  §7- ban player");
                player->sendMessage(L"§e/pardon <player>  §7- unban player");
                player->sendMessage(L"§e/tps  §7- server performance stats");
                player->sendMessage(L"§e--- Movement ---");
                player->sendMessage(L"§e/spawn  §7- teleport to world spawn");
                player->sendMessage(L"§e/sethome  §7- save current location as home");
                player->sendMessage(L"§e/home  §7- teleport to your home");
                player->sendMessage(L"§e/back  §7- return to the previous location");
                player->sendMessage(L"§e/r <message>  §7- reply to last whisper");
                return;  
            }

            case eGameCommand_Say: {
                
                std::wstring rest;
                std::getline(ss, rest);
                if (!rest.empty() && rest.front() == L' ') rest.erase(0, 1);
                std::wstring full = arg1_orig;
                if (!rest.empty()) {
                    if (!full.empty()) full += L" ";
                    full += rest;
                }
                if (full.empty()) {
                    player->sendMessage(L"§cUsage: /say <message>");
                    return;
                }
                auto chatPacket = std::make_shared<ChatPacket>(
                    L"[Server] " + player->getName(),
                    ChatPacket::e_ChatCustom, -1);
                chatPacket->m_stringArgs.push_back(full);
                
                server->getPlayers()->broadcastAll(chatPacket);
                return;  
            }

            case eGameCommand_List:
            case eGameCommand_Seed:
            case eGameCommand_Tps:
            case eGameCommand_Spawn:
            case eGameCommand_SetHome:
            case eGameCommand_Home:
            case eGameCommand_Back: {
                
                break;
            }

            case eGameCommand_Reply: {
                
                if (arg1_orig.empty()) {
                    player->sendMessage(L"§cUsage: /r <message>");
                    return;
                }
                std::wstring msg = arg1_orig;
                if (!arg2_orig.empty()) {
                    msg += L" " + arg2_orig;
                }
                if (!arg3_orig.empty()) {
                    msg += L" " + arg3_orig;
                    std::wstring restOfLine;
                    std::getline(ss, restOfLine);
                    if (!restOfLine.empty() && restOfLine.front() == L' ') {
                        restOfLine.erase(0, 1);
                    }
                    if (!restOfLine.empty()) msg += L" " + restOfLine;
                }
                dos.writeUTF(msg);
                break;
            }

            case eGameCommand_Heal:
            case eGameCommand_Feed: {
                
                dos.writeUTF(arg1_orig);  
                break;
            }

            case eGameCommand_Op:
            case eGameCommand_DeOp:
            case eGameCommand_Ban:
            case eGameCommand_Pardon: {
                if (arg1_orig.empty()) {
                    std::wstring name = L"Usage: /";
                    name += (cmd == eGameCommand_Op       ? L"op"
                             : cmd == eGameCommand_DeOp   ? L"deop"
                             : cmd == eGameCommand_Ban    ? L"ban"
                                                          : L"pardon");
                    name += L" <player>";
                    player->sendMessage(L"§c" + name);
                    return;
                }
                dos.writeUTF(arg1_orig);
                break;
            }

            case eGameCommand_Kick: {
                
                if (arg1_orig.empty()) {
                    player->sendMessage(L"§cUsage: /kick <player> [reason]");
                    return;
                }
                dos.writeUTF(arg1_orig);
                
                std::wstring reason;
                if (!arg2_orig.empty()) {
                    reason = arg2_orig;
                    if (!arg3_orig.empty()) {
                        reason += L" " + arg3_orig;
                        std::wstring restOfLine;
                        std::getline(ss, restOfLine);
                        if (!restOfLine.empty() && restOfLine.front() == L' ') {
                            restOfLine.erase(0, 1);
                        }
                        if (!restOfLine.empty()) reason += L" " + restOfLine;
                    }
                }
                dos.writeUTF(reason);
                break;
            }

            case eGameCommand_Msg: {
                
                if (arg1_orig.empty() || arg2_orig.empty()) {
                    player->sendMessage(L"§cUsage: /msg <player> <message>");
                    return;
                }
                dos.writeUTF(arg1_orig);
                std::wstring msg = arg2_orig;
                if (!arg3_orig.empty()) {
                    msg += L" " + arg3_orig;
                    std::wstring restOfLine;
                    std::getline(ss, restOfLine);
                    if (!restOfLine.empty() && restOfLine.front() == L' ') {
                        restOfLine.erase(0, 1);
                    }
                    if (!restOfLine.empty()) msg += L" " + restOfLine;
                }
                dos.writeUTF(msg);
                break;
            }

            case eGameCommand_DefaultGameMode:
            case eGameCommand_EnchantItem:
            default: {
                
                player->sendMessage(L"This command is not yet implemented");
                return;
            }
        }
        
        
        commandData = baos.toByteArray();
        
    } catch (const std::exception& e) {
        player->sendMessage(L"Error processing command");
        return;
    }
    
    
    server->getCommandDispatcher()->performCommand(player, cmd, commandData);
}

EGameCommand PlayerConnection::parseCommandName(const std::wstring& cmdName) {
    
    
    
    
    if (cmdName == L"gamemode" || cmdName == L"gm") {
        return eGameCommand_GameMode;
    } else if (cmdName == L"defaultgamemode" || cmdName == L"defaultgm") {
        return eGameCommand_DefaultGameMode;
    } else if (cmdName == L"give" || cmdName == L"g") {
        return eGameCommand_Give;
    } else if (cmdName == L"time") {
        return eGameCommand_Time;
    } else if (cmdName == L"kill") {
        return eGameCommand_Kill;
    } else if (cmdName == L"teleport" || cmdName == L"tp") {
        return eGameCommand_Teleport;
    } else if (cmdName == L"effect") {
        return eGameCommand_Effect;
    } else if (cmdName == L"enchant" || cmdName == L"enchantitem") {
        return eGameCommand_EnchantItem;
    } else if (cmdName == L"xp" || cmdName == L"experience") {
        return eGameCommand_Experience;
    } else if (cmdName == L"weather" || cmdName == L"toggledownfall") {
        return eGameCommand_ToggleDownfall;
    } else if (cmdName == L"summon" || cmdName == L"spawn") {
        return eGameCommand_Summon;
    } else if (cmdName == L"help" || cmdName == L"?") {
        return eGameCommand_Help;
    } else if (cmdName == L"say" || cmdName == L"me") {
        return eGameCommand_Say;
    } else if (cmdName == L"list" || cmdName == L"players" || cmdName == L"who") {
        return eGameCommand_List;
    } else if (cmdName == L"kick") {
        return eGameCommand_Kick;
    } else if (cmdName == L"msg" || cmdName == L"tell" || cmdName == L"w" || cmdName == L"whisper") {
        return eGameCommand_Msg;
    } else if (cmdName == L"heal") {
        return eGameCommand_Heal;
    } else if (cmdName == L"feed") {
        return eGameCommand_Feed;
    } else if (cmdName == L"seed") {
        return eGameCommand_Seed;
    } else if (cmdName == L"op") {
        return eGameCommand_Op;
    } else if (cmdName == L"deop") {
        return eGameCommand_DeOp;
    } else if (cmdName == L"ban") {
        return eGameCommand_Ban;
    } else if (cmdName == L"pardon" || cmdName == L"unban") {
        return eGameCommand_Pardon;
    } else if (cmdName == L"tps" || cmdName == L"perf") {
        return eGameCommand_Tps;
    } else if (cmdName == L"spawn") {
        return eGameCommand_Spawn;
    } else if (cmdName == L"sethome") {
        return eGameCommand_SetHome;
    } else if (cmdName == L"home") {
        return eGameCommand_Home;
    } else if (cmdName == L"back") {
        return eGameCommand_Back;
    } else if (cmdName == L"r" || cmdName == L"reply") {
        return eGameCommand_Reply;
    } else {
        
        return eGameCommand_COUNT;
    }
}

void PlayerConnection::handleAnimate(std::shared_ptr<AnimatePacket> packet) {
    player->resetLastActionTime();
    if (packet->action == AnimatePacket::SWING) {
        player->swing();
    }
}

void PlayerConnection::handlePlayerCommand(
    std::shared_ptr<PlayerCommandPacket> packet) {
    player->resetLastActionTime();
    if (packet->action == PlayerCommandPacket::START_SNEAKING) {
        player->setSneaking(true);
    } else if (packet->action == PlayerCommandPacket::STOP_SNEAKING) {
        player->setSneaking(false);
    } else if (packet->action == PlayerCommandPacket::START_SPRINTING) {
        player->setSprinting(true);
    } else if (packet->action == PlayerCommandPacket::STOP_SPRINTING) {
        player->setSprinting(false);
    } else if (packet->action == PlayerCommandPacket::STOP_SLEEPING) {
        player->stopSleepInBed(false, true, true);
        synched = false;
    } else if (packet->action == PlayerCommandPacket::RIDING_JUMP) {
        
        if ((player->riding != nullptr) &&
            player->riding->GetType() == eTYPE_HORSE) {
            std::dynamic_pointer_cast<EntityHorse>(player->riding)
                ->onPlayerJump(packet->data);
        }
    } else if (packet->action == PlayerCommandPacket::OPEN_INVENTORY) {
        
        if ((player->riding != nullptr) &&
            player->riding->instanceof(eTYPE_HORSE)) {
            std::dynamic_pointer_cast<EntityHorse>(player->riding)
                ->openInventory(player);
        }
    } else if (packet->action == PlayerCommandPacket::START_IDLEANIM) {
        player->setIsIdle(true);
    } else if (packet->action == PlayerCommandPacket::STOP_IDLEANIM) {
        player->setIsIdle(false);
    }
}

void PlayerConnection::setShowOnMaps(bool bVal) { player->setShowOnMaps(bVal); }

void PlayerConnection::handleDisconnect(
    std::shared_ptr<DisconnectPacket> packet) {
    
    
    server->getPlayers()->removePlayerFromReceiving(player);
    connection->close(DisconnectPacket::eDisconnect_Quitting);
}

int PlayerConnection::countDelayedPackets() {
    return connection->countDelayedPackets();
}

void PlayerConnection::info(const std::wstring& string) {
    
    
    
}

void PlayerConnection::warn(const std::wstring& string) {
    
    
    
}

std::wstring PlayerConnection::getConsoleName() { return player->getName(); }

void PlayerConnection::handleInteract(std::shared_ptr<InteractPacket> packet) {
    ServerLevel* level = server->getLevel(player->dimension);
    std::shared_ptr<Entity> target = level->getEntity(packet->target);
    player->resetLastActionTime();

    
    
    
    
    
    if (target != nullptr)  
                            
    {
        
        
        
        
        
        

        
        
        if (packet->action == InteractPacket::INTERACT) {
            player->interact(target);
        } else if (packet->action == InteractPacket::ATTACK) {
            if ((target->GetType() == eTYPE_ITEMENTITY) ||
                (target->GetType() == eTYPE_EXPERIENCEORB) ||
                (target->GetType() == eTYPE_ARROW) || target == player) {
                
                
                
                return;
            }
            player->attack(target);
        }
        
    }
}

bool PlayerConnection::canHandleAsyncPackets() { return true; }

void PlayerConnection::handleTexture(std::shared_ptr<TexturePacket> packet) {
    
    

    if (packet->dataBytes == 0) {
        
#if !defined(_CONTENT_PACKAGE)
        wprintf(L"Server received request for custom texture %ls\n",
                packet->textureName.c_str());
#endif
        std::uint8_t* pbData = nullptr;
        unsigned int dwBytes = 0;
        app.GetMemFileDetails(packet->textureName, &pbData, &dwBytes);

        if (dwBytes != 0) {
            send(std::shared_ptr<TexturePacket>(
                new TexturePacket(packet->textureName, pbData, dwBytes)));
        } else {
            m_texturesRequested.push_back(packet->textureName);
        }
    } else {
        
#if !defined(_CONTENT_PACKAGE)
        wprintf(L"Server received custom texture %ls\n",
                packet->textureName.c_str());
#endif
        app.AddMemoryTextureFile(packet->textureName, packet->pbData,
                                 packet->dataBytes);
        server->connection->handleTextureReceived(packet->textureName);
    }
}

void PlayerConnection::handleTextureAndGeometry(
    std::shared_ptr<TextureAndGeometryPacket> packet) {
    
    

    if (packet->dwTextureBytes == 0) {
        
#if !defined(_CONTENT_PACKAGE)
        wprintf(L"Server received request for custom texture %ls\n",
                packet->textureName.c_str());
#endif
        std::uint8_t* pbData = nullptr;
        unsigned int dwTextureBytes = 0;
        app.GetMemFileDetails(packet->textureName, &pbData, &dwTextureBytes);
        DLCSkinFile* pDLCSkinFile =
            app.m_dlcManager.getSkinFile(packet->textureName);

        if (dwTextureBytes != 0) {
            if (pDLCSkinFile) {
                if (pDLCSkinFile->getAdditionalBoxesCount() != 0) {
                    send(std::shared_ptr<TextureAndGeometryPacket>(
                        new TextureAndGeometryPacket(packet->textureName,
                                                     pbData, dwTextureBytes,
                                                     pDLCSkinFile)));
                } else {
                    send(std::shared_ptr<TextureAndGeometryPacket>(
                        new TextureAndGeometryPacket(packet->textureName,
                                                     pbData, dwTextureBytes)));
                }
            } else {
                
                
                std::vector<SKIN_BOX*>* pvSkinBoxes =
                    app.GetAdditionalSkinBoxes(packet->dwSkinID);
                unsigned int uiAnimOverrideBitmask =
                    app.GetAnimOverrideBitmask(packet->dwSkinID);

                send(std::shared_ptr<TextureAndGeometryPacket>(
                    new TextureAndGeometryPacket(packet->textureName, pbData,
                                                 dwTextureBytes, pvSkinBoxes,
                                                 uiAnimOverrideBitmask)));
            }
        } else {
            m_texturesRequested.push_back(packet->textureName);
        }
    } else {
        
#if !defined(_CONTENT_PACKAGE)
        wprintf(L"Server received custom texture %ls and geometry\n",
                packet->textureName.c_str());
#endif
        app.AddMemoryTextureFile(packet->textureName, packet->pbData,
                                 packet->dwTextureBytes);

        
        if (packet->dwBoxC != 0) {
#if !defined(_CONTENT_PACKAGE)
            wprintf(L"Adding skin boxes for skin id %X, box count %d\n",
                    packet->dwSkinID, packet->dwBoxC);
#endif
            app.SetAdditionalSkinBoxes(packet->dwSkinID, packet->BoxDataA,
                                       packet->dwBoxC);
        }
        
        app.SetAnimOverrideBitmask(packet->dwSkinID,
                                   packet->uiAnimOverrideBitmask);

        player->setCustomSkin(packet->dwSkinID);

        server->connection->handleTextureAndGeometryReceived(
            packet->textureName);
    }
}

void PlayerConnection::handleTextureReceived(const std::wstring& textureName) {
    
    
    auto it = find(m_texturesRequested.begin(), m_texturesRequested.end(),
                   textureName);
    if (it != m_texturesRequested.end()) {
        std::uint8_t* pbData = nullptr;
        unsigned int dwBytes = 0;
        app.GetMemFileDetails(textureName, &pbData, &dwBytes);

        if (dwBytes != 0) {
            send(std::shared_ptr<TexturePacket>(
                new TexturePacket(textureName, pbData, dwBytes)));
            m_texturesRequested.erase(it);
        }
    }
}

void PlayerConnection::handleTextureAndGeometryReceived(
    const std::wstring& textureName) {
    
    
    auto it = find(m_texturesRequested.begin(), m_texturesRequested.end(),
                   textureName);
    if (it != m_texturesRequested.end()) {
        std::uint8_t* pbData = nullptr;
        unsigned int dwTextureBytes = 0;
        app.GetMemFileDetails(textureName, &pbData, &dwTextureBytes);
        DLCSkinFile* pDLCSkinFile = app.m_dlcManager.getSkinFile(textureName);

        if (dwTextureBytes != 0) {
            if (pDLCSkinFile &&
                (pDLCSkinFile->getAdditionalBoxesCount() != 0)) {
                send(std::shared_ptr<TextureAndGeometryPacket>(
                    new TextureAndGeometryPacket(
                        textureName, pbData, dwTextureBytes, pDLCSkinFile)));
            } else {
                
                std::uint32_t dwSkinID = app.getSkinIdFromPath(textureName);
                std::vector<SKIN_BOX*>* pvSkinBoxes =
                    app.GetAdditionalSkinBoxes(dwSkinID);
                unsigned int uiAnimOverrideBitmask =
                    app.GetAnimOverrideBitmask(dwSkinID);

                send(std::shared_ptr<TextureAndGeometryPacket>(
                    new TextureAndGeometryPacket(textureName, pbData,
                                                 dwTextureBytes, pvSkinBoxes,
                                                 uiAnimOverrideBitmask)));
            }
            m_texturesRequested.erase(it);
        }
    }
}

void PlayerConnection::handleTextureChange(
    std::shared_ptr<TextureChangePacket> packet) {
    switch (packet->action) {
        case TextureChangePacket::e_TextureChange_Skin:
            player->setCustomSkin(app.getSkinIdFromPath(packet->path));
#if !defined(_CONTENT_PACKAGE)
            wprintf(L"Skin for server player %ls has changed to %ls (%d)\n",
                    player->name.c_str(), player->customTextureUrl.c_str(),
                    player->getPlayerDefaultSkin());
#endif
            break;
        case TextureChangePacket::e_TextureChange_Cape:
            player->setCustomCape(Player::getCapeIdFromPath(packet->path));
            
#if !defined(_CONTENT_PACKAGE)
            wprintf(L"Cape for server player %ls has changed to %ls\n",
                    player->name.c_str(), player->customTextureUrl2.c_str());
#endif
            break;
    }
    if (!packet->path.empty() &&
        packet->path.substr(0, 3).compare(L"def") != 0 &&
        !app.IsFileInMemoryTextures(packet->path)) {
        if (server->connection->addPendingTextureRequest(packet->path)) {
#if !defined(_CONTENT_PACKAGE)
            wprintf(
                L"Sending texture packet to get custom skin %ls from player "
                L"%ls\n",
                packet->path.c_str(), player->name.c_str());
#endif
            send(std::shared_ptr<TexturePacket>(
                new TexturePacket(packet->path, nullptr, 0)));
        }
    } else if (!packet->path.empty() &&
               app.IsFileInMemoryTextures(packet->path)) {
        
        app.AddMemoryTextureFile(packet->path, nullptr, 0);
    }
    server->getPlayers()->broadcastAll(
        std::shared_ptr<TextureChangePacket>(
            new TextureChangePacket(player, packet->action, packet->path)),
        player->dimension);
}

void PlayerConnection::handleTextureAndGeometryChange(
    std::shared_ptr<TextureAndGeometryChangePacket> packet) {
    player->setCustomSkin(app.getSkinIdFromPath(packet->path));
#if !defined(_CONTENT_PACKAGE)
    wprintf(
        L"PlayerConnection::handleTextureAndGeometryChange - Skin for server "
        L"player %ls has changed to %ls (%d)\n",
        player->name.c_str(), player->customTextureUrl.c_str(),
        player->getPlayerDefaultSkin());
#endif

    if (!packet->path.empty() &&
        packet->path.substr(0, 3).compare(L"def") != 0 &&
        !app.IsFileInMemoryTextures(packet->path)) {
        if (server->connection->addPendingTextureRequest(packet->path)) {
#if !defined(_CONTENT_PACKAGE)
            wprintf(
                L"Sending texture packet to get custom skin %ls from player "
                L"%ls\n",
                packet->path.c_str(), player->name.c_str());
#endif
            send(std::shared_ptr<TextureAndGeometryPacket>(
                new TextureAndGeometryPacket(packet->path, nullptr, 0)));
        }
    } else if (!packet->path.empty() &&
               app.IsFileInMemoryTextures(packet->path)) {
        
        app.AddMemoryTextureFile(packet->path, nullptr, 0);

        player->setCustomSkin(packet->dwSkinID);

        
        
        
        
    }
    server->getPlayers()->broadcastAll(
        std::shared_ptr<TextureAndGeometryChangePacket>(
            new TextureAndGeometryChangePacket(player, packet->path)),
        player->dimension);
}

void PlayerConnection::handleServerSettingsChanged(
    std::shared_ptr<ServerSettingsChangedPacket> packet) {
    if (packet->action == ServerSettingsChangedPacket::HOST_IN_GAME_SETTINGS) {
        
        

        INetworkPlayer* networkPlayer = getNetworkPlayer();
        if ((networkPlayer != nullptr && networkPlayer->IsHost()) ||
            player->isModerator()) {
            app.SetGameHostOption(
                eGameHostOption_FireSpreads,
                app.GetGameHostOption(packet->data,
                                      eGameHostOption_FireSpreads));
            app.SetGameHostOption(
                eGameHostOption_TNT,
                app.GetGameHostOption(packet->data, eGameHostOption_TNT));
            app.SetGameHostOption(
                eGameHostOption_MobGriefing,
                app.GetGameHostOption(packet->data,
                                      eGameHostOption_MobGriefing));
            app.SetGameHostOption(
                eGameHostOption_KeepInventory,
                app.GetGameHostOption(packet->data,
                                      eGameHostOption_KeepInventory));
            app.SetGameHostOption(
                eGameHostOption_DoMobSpawning,
                app.GetGameHostOption(packet->data,
                                      eGameHostOption_DoMobSpawning));
            app.SetGameHostOption(
                eGameHostOption_DoMobLoot,
                app.GetGameHostOption(packet->data, eGameHostOption_DoMobLoot));
            app.SetGameHostOption(
                eGameHostOption_DoTileDrops,
                app.GetGameHostOption(packet->data,
                                      eGameHostOption_DoTileDrops));
            app.SetGameHostOption(
                eGameHostOption_DoDaylightCycle,
                app.GetGameHostOption(packet->data,
                                      eGameHostOption_DoDaylightCycle));
            app.SetGameHostOption(
                eGameHostOption_NaturalRegeneration,
                app.GetGameHostOption(packet->data,
                                      eGameHostOption_NaturalRegeneration));

            server->getPlayers()->broadcastAll(
                std::shared_ptr<ServerSettingsChangedPacket>(
                    new ServerSettingsChangedPacket(
                        ServerSettingsChangedPacket::HOST_IN_GAME_SETTINGS,
                        app.GetGameHostOption(eGameHostOption_All))));

            
            g_NetworkManager.UpdateAndSetGameSessionData();
        }
    }
}

void PlayerConnection::handleKickPlayer(
    std::shared_ptr<KickPlayerPacket> packet) {
    INetworkPlayer* networkPlayer = getNetworkPlayer();
    if ((networkPlayer != nullptr && networkPlayer->IsHost()) ||
        player->isModerator()) {
        server->getPlayers()->kickPlayerByShortId(packet->m_networkSmallId);
    }
}

void PlayerConnection::handleGameCommand(
    std::shared_ptr<GameCommandPacket> packet) {
    MinecraftServer::getInstance()->getCommandDispatcher()->performCommand(
        player, packet->command, packet->data);
}

void PlayerConnection::handleClientCommand(
    std::shared_ptr<ClientCommandPacket> packet) {
    player->resetLastActionTime();
    if (packet->action == ClientCommandPacket::PERFORM_RESPAWN) {
        if (player->wonGame) {
            player = server->getPlayers()->respawn(
                player, player->m_enteredEndExitPortal ? 0 : player->dimension,
                true);
        }
        
        
        
        
        
        
        
        
        
        
        
        

        
        
        
        
        
        else {
            if (player->getHealth() > 0) return;
            player = server->getPlayers()->respawn(player, 0, false);
        }
    }
}

void PlayerConnection::handleRespawn(std::shared_ptr<RespawnPacket> packet) {}

void PlayerConnection::handleContainerClose(
    std::shared_ptr<ContainerClosePacket> packet) {
    player->doCloseContainer();
}

#if !defined(_CONTENT_PACKAGE)
void PlayerConnection::handleContainerSetSlot(
    std::shared_ptr<ContainerSetSlotPacket> packet) {
    if (packet->containerId == AbstractContainerMenu::CONTAINER_ID_CARRIED) {
        player->inventory->setCarried(packet->item);
    } else {
        if (packet->containerId ==
                AbstractContainerMenu::CONTAINER_ID_INVENTORY &&
            packet->slot >= 36 && packet->slot < 36 + 9) {
            std::shared_ptr<ItemInstance> lastItem =
                player->inventoryMenu->getSlot(packet->slot)->getItem();
            if (packet->item != nullptr) {
                if (lastItem == nullptr ||
                    lastItem->count < packet->item->count) {
                    packet->item->popTime = Inventory::POP_TIME_DURATION;
                }
            }
            player->inventoryMenu->setItem(packet->slot, packet->item);
            player->ignoreSlotUpdateHack = true;
            player->containerMenu->broadcastChanges();
            player->broadcastCarriedItem();
            player->ignoreSlotUpdateHack = false;
        } else if (packet->containerId == player->containerMenu->containerId) {
            player->containerMenu->setItem(packet->slot, packet->item);
            player->ignoreSlotUpdateHack = true;
            player->containerMenu->broadcastChanges();
            player->broadcastCarriedItem();
            player->ignoreSlotUpdateHack = false;
        }
    }
}
#endif

void PlayerConnection::handleContainerClick(
    std::shared_ptr<ContainerClickPacket> packet) {
    player->resetLastActionTime();
    if (player->containerMenu->containerId == packet->containerId &&
        player->containerMenu->isSynched(player)) {
        std::shared_ptr<ItemInstance> clicked = player->containerMenu->clicked(
            packet->slotNum, packet->buttonNum, packet->clickType, player);

        if (ItemInstance::matches(packet->item, clicked)) {
            
            player->connection->send(std::make_shared<ContainerAckPacket>(
                packet->containerId, packet->uid, true));
            player->ignoreSlotUpdateHack = true;
            player->containerMenu->broadcastChanges();
            player->broadcastCarriedItem();
            player->ignoreSlotUpdateHack = false;
        } else {
            
            expectedAcks[player->containerMenu->containerId] = packet->uid;
            player->connection->send(std::make_shared<ContainerAckPacket>(
                packet->containerId, packet->uid, false));
            player->containerMenu->setSynched(player, false);

            std::vector<std::shared_ptr<ItemInstance> > items;
            for (unsigned int i = 0; i < player->containerMenu->slots.size();
                 i++) {
                items.push_back(player->containerMenu->slots.at(i)->getItem());
            }
            player->refreshContainer(player->containerMenu, &items);

            
        }
    }
}

void PlayerConnection::handleContainerButtonClick(
    std::shared_ptr<ContainerButtonClickPacket> packet) {
    player->resetLastActionTime();
    if (player->containerMenu->containerId == packet->containerId &&
        player->containerMenu->isSynched(player)) {
        player->containerMenu->clickMenuButton(player, packet->buttonId);
        player->containerMenu->broadcastChanges();
    }
}

void PlayerConnection::handleSetCreativeModeSlot(
    std::shared_ptr<SetCreativeModeSlotPacket> packet) {
    if (player->gameMode->isCreative()) {
        bool drop = packet->slotNum < 0;
        std::shared_ptr<ItemInstance> item = packet->item;

        if (item != nullptr && item->id == Item::map_Id) {
            int mapScale = 3;
#if defined(_LARGE_WORLDS)
            int scale = MapItemSavedData::MAP_SIZE * 2 * (1 << mapScale);
            int centreXC = (int)(Math::round(player->x / scale) * scale);
            int centreZC = (int)(Math::round(player->z / scale) * scale);
#else
            
            
            int centreXC = 0;
            int centreZC = 0;
#endif
            item->setAuxValue(player->level->getAuxValueForMap(
                player->getXuid(), player->dimension, centreXC, centreZC,
                mapScale));

            std::shared_ptr<MapItemSavedData> data =
                MapItem::getSavedData(item->getAuxValue(), player->level);
            
            
            wchar_t buf[64];
            swprintf(buf, 64, L"map_%d", item->getAuxValue());
            std::wstring id = std::wstring(buf);
            if (data == nullptr) {
                data = std::make_shared<MapItemSavedData>(id);
            }
            player->level->setSavedData(id, (std::shared_ptr<SavedData>)data);

            data->scale = mapScale;
            
            
            data->x = centreXC;
            data->z = centreZC;
            data->dimension = (std::uint8_t)player->level->dimension->id;
            data->setDirty();
        }

        bool validSlot = (packet->slotNum >= InventoryMenu::CRAFT_SLOT_START &&
                          packet->slotNum < (InventoryMenu::USE_ROW_SLOT_START +
                                             Inventory::getSelectionSize()));
        bool validItem = item == nullptr ||
                         (item->id < Item::items.size() && item->id >= 0 &&
                          Item::items[item->id] != nullptr);
        bool validData =
            item == nullptr ||
            (item->getAuxValue() >= 0 && item->count > 0 && item->count <= 64);

        if (validSlot && validItem && validData) {
            if (item == nullptr) {
                player->inventoryMenu->setItem(packet->slotNum, nullptr);
            } else {
                player->inventoryMenu->setItem(packet->slotNum, item);
            }
            player->inventoryMenu->setSynched(player, true);
            
            
            
        } else if (drop && validItem && validData) {
            if (dropSpamTickCount < SharedConstants::TICKS_PER_SECOND * 10) {
                dropSpamTickCount += SharedConstants::TICKS_PER_SECOND;
                
                std::shared_ptr<ItemEntity> dropped = player->drop(item);
                if (dropped != nullptr) {
                    dropped->setShortLifeTime();
                }
            }
        }

        if (item != nullptr && item->id == Item::map_Id) {
            
            
            
            
            std::vector<std::shared_ptr<ItemInstance> > items;
            for (unsigned int i = 0; i < player->inventoryMenu->slots.size();
                 i++) {
                items.push_back(player->inventoryMenu->slots.at(i)->getItem());
            }
            player->refreshContainer(player->inventoryMenu, &items);
        }
    }
}

void PlayerConnection::handleContainerAck(
    std::shared_ptr<ContainerAckPacket> packet) {
    auto it = expectedAcks.find(player->containerMenu->containerId);

    if (it != expectedAcks.end() && packet->uid == it->second &&
        player->containerMenu->containerId == packet->containerId &&
        !player->containerMenu->isSynched(player)) {
        player->containerMenu->setSynched(player, true);
    }
}

void PlayerConnection::handleSignUpdate(
    std::shared_ptr<SignUpdatePacket> packet) {
    player->resetLastActionTime();
    app.DebugPrintf("PlayerConnection::handleSignUpdate\n");

    ServerLevel* level = server->getLevel(player->dimension);
    if (level->hasChunkAt(packet->x, packet->y, packet->z)) {
        std::shared_ptr<TileEntity> te =
            level->getTileEntity(packet->x, packet->y, packet->z);

        if (std::dynamic_pointer_cast<SignTileEntity>(te) != nullptr) {
            std::shared_ptr<SignTileEntity> ste =
                std::dynamic_pointer_cast<SignTileEntity>(te);
            if (!ste->isEditable() || ste->getPlayerWhoMayEdit() != player) {
                server->warn(L"Player " + player->getName() +
                             L" just tried to change non-editable sign");
                return;
            }
        }

        
        if (std::dynamic_pointer_cast<SignTileEntity>(te) != nullptr) {
            int x = packet->x;
            int y = packet->y;
            int z = packet->z;
            std::shared_ptr<SignTileEntity> ste =
                std::dynamic_pointer_cast<SignTileEntity>(te);
            for (int i = 0; i < 4; i++) {
                std::wstring lineText = packet->lines[i].substr(0, 15);
                ste->SetMessage(i, lineText);
            }
            ste->SetVerified(false);
            ste->setChanged();
            level->sendTileUpdated(x, y, z);
        }
    }
}

void PlayerConnection::handleKeepAlive(
    std::shared_ptr<KeepAlivePacket> packet) {
    if (packet->id == lastKeepAliveId) {
        int time = (int)(System::nanoTime() / 1000000 - lastKeepAliveTime);
        player->latency = (player->latency * 3 + time) / 4;
    }
}

void PlayerConnection::handlePlayerInfo(
    std::shared_ptr<PlayerInfoPacket> packet) {
    
    

    INetworkPlayer* networkPlayer = getNetworkPlayer();
    if ((networkPlayer != nullptr && networkPlayer->IsHost()) ||
        player->isModerator()) {
        std::shared_ptr<ServerPlayer> serverPlayer;
        
        for (auto it = server->getPlayers()->players.begin();
             it != server->getPlayers()->players.end(); ++it) {
            std::shared_ptr<ServerPlayer> checkingPlayer = *it;
            if (checkingPlayer->connection->getNetworkPlayer() != nullptr &&
                checkingPlayer->connection->getNetworkPlayer()->GetSmallId() ==
                    packet->m_networkSmallId) {
                serverPlayer = checkingPlayer;
                break;
            }
        }

        if (serverPlayer != nullptr) {
            unsigned int origPrivs = serverPlayer->getAllPlayerGamePrivileges();

            bool trustPlayers =
                app.GetGameHostOption(eGameHostOption_TrustPlayers) != 0;
            bool cheats =
                app.GetGameHostOption(eGameHostOption_CheatsEnabled) != 0;
            if (serverPlayer == player) {
                GameType* gameType =
                    Player::getPlayerGamePrivilege(
                        packet->m_playerPrivileges,
                        Player::ePlayerGamePrivilege_CreativeMode)
                        ? GameType::CREATIVE
                        : GameType::SURVIVAL;
                gameType = LevelSettings::validateGameType(gameType->getId());
                if (serverPlayer->gameMode->getGameModeForPlayer() !=
                    gameType) {
#if !defined(_CONTENT_PACKAGE)
                    wprintf(L"Setting %ls to game mode %d\n",
                            serverPlayer->name.c_str(), gameType);
#endif
                    serverPlayer->setPlayerGamePrivilege(
                        Player::ePlayerGamePrivilege_CreativeMode,
                        Player::getPlayerGamePrivilege(
                            packet->m_playerPrivileges,
                            Player::ePlayerGamePrivilege_CreativeMode));
                    serverPlayer->gameMode->setGameModeForPlayer(gameType);
                    serverPlayer->connection->send(
                        std::make_shared<GameEventPacket>(
                            GameEventPacket::CHANGE_GAME_MODE,
                            gameType->getId()));
                } else {
#if !defined(_CONTENT_PACKAGE)
                    wprintf(L"%ls already has game mode %d\n",
                            serverPlayer->name.c_str(), gameType);
#endif
                }
                if (cheats) {
                    
                    bool canBeInvisible =
                        Player::getPlayerGamePrivilege(
                            origPrivs,
                            Player::ePlayerGamePrivilege_CanToggleInvisible) !=
                        0;
                    if (canBeInvisible)
                        serverPlayer->setPlayerGamePrivilege(
                            Player::ePlayerGamePrivilege_Invisible,
                            Player::getPlayerGamePrivilege(
                                packet->m_playerPrivileges,
                                Player::ePlayerGamePrivilege_Invisible));
                    if (canBeInvisible)
                        serverPlayer->setPlayerGamePrivilege(
                            Player::ePlayerGamePrivilege_Invulnerable,
                            Player::getPlayerGamePrivilege(
                                packet->m_playerPrivileges,
                                Player::ePlayerGamePrivilege_Invulnerable));

                    bool inCreativeMode =
                        Player::getPlayerGamePrivilege(
                            origPrivs,
                            Player::ePlayerGamePrivilege_CreativeMode) != 0;
                    if (!inCreativeMode) {
                        bool canFly = Player::getPlayerGamePrivilege(
                            origPrivs,
                            Player::ePlayerGamePrivilege_CanToggleFly);
                        bool canChangeHunger = Player::getPlayerGamePrivilege(
                            origPrivs,
                            Player::
                                ePlayerGamePrivilege_CanToggleClassicHunger);

                        if (canFly)
                            serverPlayer->setPlayerGamePrivilege(
                                Player::ePlayerGamePrivilege_CanFly,
                                Player::getPlayerGamePrivilege(
                                    packet->m_playerPrivileges,
                                    Player::ePlayerGamePrivilege_CanFly));
                        if (canChangeHunger)
                            serverPlayer->setPlayerGamePrivilege(
                                Player::ePlayerGamePrivilege_ClassicHunger,
                                Player::getPlayerGamePrivilege(
                                    packet->m_playerPrivileges,
                                    Player::
                                        ePlayerGamePrivilege_ClassicHunger));
                    }
                }
            } else {
                
                if (!trustPlayers &&
                    !serverPlayer->connection->getNetworkPlayer()->IsHost()) {
                    serverPlayer->setPlayerGamePrivilege(
                        Player::ePlayerGamePrivilege_CannotMine,
                        Player::getPlayerGamePrivilege(
                            packet->m_playerPrivileges,
                            Player::ePlayerGamePrivilege_CannotMine));
                    serverPlayer->setPlayerGamePrivilege(
                        Player::ePlayerGamePrivilege_CannotBuild,
                        Player::getPlayerGamePrivilege(
                            packet->m_playerPrivileges,
                            Player::ePlayerGamePrivilege_CannotBuild));
                    serverPlayer->setPlayerGamePrivilege(
                        Player::ePlayerGamePrivilege_CannotAttackPlayers,
                        Player::getPlayerGamePrivilege(
                            packet->m_playerPrivileges,
                            Player::ePlayerGamePrivilege_CannotAttackPlayers));
                    serverPlayer->setPlayerGamePrivilege(
                        Player::ePlayerGamePrivilege_CannotAttackAnimals,
                        Player::getPlayerGamePrivilege(
                            packet->m_playerPrivileges,
                            Player::ePlayerGamePrivilege_CannotAttackAnimals));
                    serverPlayer->setPlayerGamePrivilege(
                        Player::ePlayerGamePrivilege_CanUseDoorsAndSwitches,
                        Player::getPlayerGamePrivilege(
                            packet->m_playerPrivileges,
                            Player::
                                ePlayerGamePrivilege_CanUseDoorsAndSwitches));
                    serverPlayer->setPlayerGamePrivilege(
                        Player::ePlayerGamePrivilege_CanUseContainers,
                        Player::getPlayerGamePrivilege(
                            packet->m_playerPrivileges,
                            Player::ePlayerGamePrivilege_CanUseContainers));
                }

                if (networkPlayer->IsHost()) {
                    if (cheats) {
                        serverPlayer->setPlayerGamePrivilege(
                            Player::ePlayerGamePrivilege_CanToggleInvisible,
                            Player::getPlayerGamePrivilege(
                                packet->m_playerPrivileges,
                                Player::
                                    ePlayerGamePrivilege_CanToggleInvisible));
                        serverPlayer->setPlayerGamePrivilege(
                            Player::ePlayerGamePrivilege_CanToggleFly,
                            Player::getPlayerGamePrivilege(
                                packet->m_playerPrivileges,
                                Player::ePlayerGamePrivilege_CanToggleFly));
                        serverPlayer->setPlayerGamePrivilege(
                            Player::ePlayerGamePrivilege_CanToggleClassicHunger,
                            Player::getPlayerGamePrivilege(
                                packet->m_playerPrivileges,
                                Player::
                                    ePlayerGamePrivilege_CanToggleClassicHunger));
                        serverPlayer->setPlayerGamePrivilege(
                            Player::ePlayerGamePrivilege_CanTeleport,
                            Player::getPlayerGamePrivilege(
                                packet->m_playerPrivileges,
                                Player::ePlayerGamePrivilege_CanTeleport));
                    }
                    serverPlayer->setPlayerGamePrivilege(
                        Player::ePlayerGamePrivilege_Op,
                        Player::getPlayerGamePrivilege(
                            packet->m_playerPrivileges,
                            Player::ePlayerGamePrivilege_Op));
                }
            }

            server->getPlayers()->broadcastAll(
                std::shared_ptr<PlayerInfoPacket>(
                    new PlayerInfoPacket(serverPlayer)));
        }
    }
}

bool PlayerConnection::isServerPacketListener() { return true; }

void PlayerConnection::handlePlayerAbilities(
    std::shared_ptr<PlayerAbilitiesPacket> playerAbilitiesPacket) {
    player->abilities.flying =
        playerAbilitiesPacket->isFlying() && player->abilities.mayfly;
}



















void PlayerConnection::handleClientInformation(
    std::shared_ptr<ClientInformationPacket> packet) {
    
    
    
    
    
    int requested = clampViewDistance(packet->viewDistance);
    m_pendingClientViewDistance.store(requested, std::memory_order_relaxed);
}

void PlayerConnection::handleCustomPayload(
    std::shared_ptr<CustomPayloadPacket> customPayloadPacket) {
    if (CustomPayloadPacket::TRADER_SELECTION_PACKET.compare(
            customPayloadPacket->identifier) == 0) {
        ByteArrayInputStream bais(customPayloadPacket->data);
        DataInputStream input(&bais);
        int selection = input.readInt();

        AbstractContainerMenu* menu = player->containerMenu;
        if (dynamic_cast<MerchantMenu*>(menu)) {
            ((MerchantMenu*)menu)->setSelectionHint(selection);
        }
    } else if (CustomPayloadPacket::SET_ADVENTURE_COMMAND_PACKET.compare(
                   customPayloadPacket->identifier) == 0) {
        if (!server->isCommandBlockEnabled()) {
            app.DebugPrintf("Command blocks not enabled");
            
        } else if (player->hasPermission(eGameCommand_Effect) &&
                   player->abilities.instabuild) {
            ByteArrayInputStream bais(customPayloadPacket->data);
            DataInputStream input(&bais);
            int x = input.readInt();
            int y = input.readInt();
            int z = input.readInt();
            std::wstring command = Packet::readUtf(&input, 256);

            std::shared_ptr<TileEntity> tileEntity =
                player->level->getTileEntity(x, y, z);
            std::shared_ptr<CommandBlockEntity> cbe =
                std::dynamic_pointer_cast<CommandBlockEntity>(tileEntity);
            if (tileEntity != nullptr && cbe != nullptr) {
                cbe->setCommand(command);
                player->level->sendTileUpdated(x, y, z);
                
                
            }
        } else {
            
        }
    } else if (CustomPayloadPacket::SET_BEACON_PACKET.compare(
                   customPayloadPacket->identifier) == 0) {
        if (dynamic_cast<BeaconMenu*>(player->containerMenu) != nullptr) {
            ByteArrayInputStream bais(customPayloadPacket->data);
            DataInputStream input(&bais);
            int primary = input.readInt();
            int secondary = input.readInt();

            BeaconMenu* beaconMenu = (BeaconMenu*)player->containerMenu;
            Slot* slot = beaconMenu->getSlot(0);
            if (slot->hasItem()) {
                slot->remove(1);
                std::shared_ptr<BeaconTileEntity> beacon =
                    beaconMenu->getBeacon();
                beacon->setPrimaryPower(primary);
                beacon->setSecondaryPower(secondary);
                beacon->setChanged();
            }
        }
    } else if (CustomPayloadPacket::SET_ITEM_NAME_PACKET.compare(
                   customPayloadPacket->identifier) == 0) {
        AnvilMenu* menu = dynamic_cast<AnvilMenu*>(player->containerMenu);
        if (menu) {
            if (customPayloadPacket->data.empty()) {
                menu->setItemName(L"");
            } else {
                ByteArrayInputStream bais(customPayloadPacket->data);
                DataInputStream dis(&bais);
                std::wstring name = dis.readUTF();
                if (name.length() <= 30) {
                    menu->setItemName(name);
                }
            }
        }
    }
}

bool PlayerConnection::isDisconnected() { return done; }



void PlayerConnection::handleDebugOptions(
    std::shared_ptr<DebugOptionsPacket> packet) {
    
    
    player->SetDebugOptions(packet->m_uiVal);
}

void PlayerConnection::handleCraftItem(
    std::shared_ptr<CraftItemPacket> packet) {
    int iRecipe = packet->recipe;

    if (iRecipe == -1) return;

    Recipy::INGREDIENTS_REQUIRED* pRecipeIngredientsRequired =
        Recipes::getInstance()->getRecipeIngredientsArray();
    std::shared_ptr<ItemInstance> pTempItemInst =
        pRecipeIngredientsRequired[iRecipe].pRecipy->assemble(nullptr);

    if (app.DebugSettingsOn() &&
        (player->GetDebugOptions() & (1L << eDebugSetting_CraftAnything))) {
        pTempItemInst->onCraftedBy(
            player->level,
            std::dynamic_pointer_cast<Player>(player->shared_from_this()),
            pTempItemInst->count);
        if (player->inventory->add(pTempItemInst) == false) {
            
            player->drop(pTempItemInst);
        }
    } else if (pTempItemInst->id == Item::fireworksCharge_Id ||
               pTempItemInst->id == Item::fireworks_Id) {
        CraftingMenu* menu = (CraftingMenu*)player->containerMenu;
        player->openFireworks(menu->getX(), menu->getY(), menu->getZ());
    } else {
        
        
        
        
        pTempItemInst->onCraftedBy(
            player->level,
            std::dynamic_pointer_cast<Player>(player->shared_from_this()),
            pTempItemInst->count);

        
        for (int i = 0; i < pRecipeIngredientsRequired[iRecipe].iIngC; i++) {
            for (int j = 0; j < pRecipeIngredientsRequired[iRecipe].iIngValA[i];
                 j++) {
                std::shared_ptr<ItemInstance> ingItemInst = nullptr;
                
                if (pRecipeIngredientsRequired[iRecipe].iIngAuxValA[i] !=
                    Recipes::ANY_AUX_VALUE) {
                    ingItemInst = player->inventory->getResourceItem(
                        pRecipeIngredientsRequired[iRecipe].iIngIDA[i],
                        pRecipeIngredientsRequired[iRecipe].iIngAuxValA[i]);
                    player->inventory->removeResource(
                        pRecipeIngredientsRequired[iRecipe].iIngIDA[i],
                        pRecipeIngredientsRequired[iRecipe].iIngAuxValA[i]);
                } else {
                    ingItemInst = player->inventory->getResourceItem(
                        pRecipeIngredientsRequired[iRecipe].iIngIDA[i]);
                    player->inventory->removeResource(
                        pRecipeIngredientsRequired[iRecipe].iIngIDA[i]);
                }

                
                
                if (ingItemInst != nullptr) {
                    if (ingItemInst->getItem()->hasCraftingRemainingItem()) {
                        
                        player->inventory->add(std::make_shared<ItemInstance>(
                            ingItemInst->getItem()
                                ->getCraftingRemainingItem()));
                    }
                }
            }
        }

        
        
        if (player->inventory->add(pTempItemInst) == false) {
            
            player->drop(pTempItemInst);
        }

        if (pTempItemInst->id == Item::map_Id) {
            
            
            
            
            std::vector<std::shared_ptr<ItemInstance> > items;
            for (unsigned int i = 0; i < player->containerMenu->slots.size();
                 i++) {
                items.push_back(player->containerMenu->slots.at(i)->getItem());
            }
            player->refreshContainer(player->containerMenu, &items);
        } else {
            
            
            
            
            
            
            player->ignoreSlotUpdateHack = true;
            player->containerMenu->broadcastChanges();
            player->broadcastCarriedItem();
            player->ignoreSlotUpdateHack = false;
        }
    }

    
    switch (pTempItemInst->id) {
        case Tile::workBench_Id:
            player->awardStat(GenericStats::buildWorkbench(),
                              GenericStats::param_buildWorkbench());
            break;
        case Item::pickAxe_wood_Id:
            player->awardStat(GenericStats::buildPickaxe(),
                              GenericStats::param_buildPickaxe());
            break;
        case Tile::furnace_Id:
            player->awardStat(GenericStats::buildFurnace(),
                              GenericStats::param_buildFurnace());
            break;
        case Item::hoe_wood_Id:
            player->awardStat(GenericStats::buildHoe(),
                              GenericStats::param_buildHoe());
            break;
        case Item::bread_Id:
            player->awardStat(GenericStats::makeBread(),
                              GenericStats::param_makeBread());
            break;
        case Item::cake_Id:
            player->awardStat(GenericStats::bakeCake(),
                              GenericStats::param_bakeCake());
            break;
        case Item::pickAxe_stone_Id:
            player->awardStat(GenericStats::buildBetterPickaxe(),
                              GenericStats::param_buildBetterPickaxe());
            break;
        case Item::sword_wood_Id:
            player->awardStat(GenericStats::buildSword(),
                              GenericStats::param_buildSword());
            break;
        case Tile::dispenser_Id:
            player->awardStat(GenericStats::dispenseWithThis(),
                              GenericStats::param_dispenseWithThis());
            break;
        case Tile::enchantTable_Id:
            player->awardStat(GenericStats::enchantments(),
                              GenericStats::param_enchantments());
            break;
        case Tile::bookshelf_Id:
            player->awardStat(GenericStats::bookcase(),
                              GenericStats::param_bookcase());
            break;
    }
    
    
}

void PlayerConnection::handleTradeItem(
    std::shared_ptr<TradeItemPacket> packet) {
    if (player->containerMenu->containerId == packet->containerId) {
        MerchantMenu* menu = (MerchantMenu*)player->containerMenu;

        MerchantRecipeList* offers = menu->getMerchant()->getOffers(player);

        if (offers) {
            int selectedShopItem = packet->offer;
            if (selectedShopItem < offers->size()) {
                MerchantRecipe* activeRecipe = offers->at(selectedShopItem);
                if (!activeRecipe->isDeprecated()) {
                    
                    std::shared_ptr<ItemInstance> buyAItem =
                        activeRecipe->getBuyAItem();
                    std::shared_ptr<ItemInstance> buyBItem =
                        activeRecipe->getBuyBItem();

                    int buyAMatches = player->inventory->countMatches(buyAItem);
                    int buyBMatches = player->inventory->countMatches(buyBItem);
                    if ((buyAItem != nullptr &&
                         buyAMatches >= buyAItem->count) &&
                        (buyBItem == nullptr ||
                         buyBMatches >= buyBItem->count)) {
                        menu->getMerchant()->notifyTrade(activeRecipe);

                        
                        player->inventory->removeResources(buyAItem);
                        player->inventory->removeResources(buyBItem);

                        
                        std::shared_ptr<ItemInstance> result =
                            activeRecipe->getSellItem()->copy();

                        
                        player->awardStat(
                            GenericStats::itemsBought(result->getItem()->id),
                            GenericStats::param_itemsBought(
                                result->getItem()->id, result->getAuxValue(),
                                result->GetCount()));

                        if (!player->inventory->add(result)) {
                            player->drop(result);
                        }
                    }
                }
            }
        }
    }
}

INetworkPlayer* PlayerConnection::getNetworkPlayer() {
    if (connection != nullptr && connection->getSocket() != nullptr)
        return connection->getSocket()->getPlayer();
    else
        return nullptr;
}

bool PlayerConnection::isLocal() {
    if (connection->getSocket() == nullptr) {
        return false;
    } else {
        bool isLocal = connection->getSocket()->isLocal();
        return connection->getSocket()->isLocal();
    }
}

bool PlayerConnection::isGuest() {
    if (connection->getSocket() == nullptr) {
        return false;
    } else {
        INetworkPlayer* networkPlayer = connection->getSocket()->getPlayer();
        bool isGuest = false;
        if (networkPlayer != nullptr) {
            isGuest = networkPlayer->IsGuest() == true;
        }
        return isGuest;
    }
}
