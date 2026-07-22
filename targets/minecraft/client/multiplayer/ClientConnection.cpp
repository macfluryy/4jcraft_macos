#include "ClientConnection.h"

#include <assert.h>
#include <stdio.h>
#include <wchar.h>

#include <cmath>
#include <cstdint>
#include <format>
#include <limits>
#include <typeinfo>
#include <unordered_set>

#include "platform/PlatformTypes.h"
#include "platform/sdl2/Input.h"
#include "platform/sdl2/Profile.h"
#include "app/common/App_enums.h"
#include "app/common/App_structs.h"
#include "app/common/src/ConsoleGameMode.h"
#include "app/common/src/DLC/DLCManager.h"
#include "app/common/src/DLC/DLCPack.h"
#include "app/common/src/DLC/DLCSkinFile.h"
#include "app/common/src/GameRules/LevelRules/RuleDefinitions/GameRuleDefinition.h"
#include "app/common/src/Network/GameNetworkManager.h"
#include "app/common/src/Network/NetworkPlayerInterface.h"
#include "app/common/src/Network/Socket.h"
#include "app/common/src/Tutorial/FullTutorialMode.h"
#include "app/common/src/Tutorial/Tutorial.h"
#include "app/common/src/Tutorial/TutorialEnum.h"
#include "app/common/src/Tutorial/TutorialMode.h"
#include "app/common/src/UI/All Platforms/UIEnums.h"
#include "app/common/src/UI/All Platforms/UIStructs.h"
#include "app/common/src/UI/Scenes/In-Game Menu Screens/Containers/UIScene_TradingMenu.h"
#include "app/mac/MacGame.h"
#include "app/mac/Mac_UIController.h"
#include "app/mac/Stubs/winapi_stubs.h"
#include "MultiPlayerLevel.h"
#include "ReceivingLevelScreen.h"
#include "util/Timer.h"
#include "util/StringHelpers.h"
#include "java/Class.h"
#include "java/InputOutputStream/ByteArrayInputStream.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "java/Random.h"
#include "minecraft/Pos.h"
#include "minecraft/SharedConstants.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/Options.h"
#include "minecraft/client/ProgressRenderer.h"
#include "minecraft/client/User.h"
#include "minecraft/client/gui/Gui.h"
#include "minecraft/client/multiplayer/MultiPlayerGameMode.h"
#include "minecraft/client/multiplayer/MultiPlayerLocalPlayer.h"
#include "minecraft/client/player/Input.h"
#include "minecraft/client/particle/CritParticle.h"
#include "minecraft/client/particle/ParticleEngine.h"
#include "minecraft/client/particle/TakeAnimationParticle.h"
#include "minecraft/client/player/LocalPlayer.h"
#include "minecraft/client/player/RemotePlayer.h"
#include "minecraft/client/renderer/LevelRenderer.h"
#include "minecraft/client/skins/DLCTexturePack.h"
#include "minecraft/client/skins/TexturePackRepository.h"
#include "minecraft/client/gui/inventory/MerchantScreen.h"
#include "minecraft/core/particles/ParticleTypes.h"
#include "minecraft/network/packet/AddEntityPacket.h"
#include "minecraft/network/packet/AddExperienceOrbPacket.h"
#include "minecraft/network/packet/AddGlobalEntityPacket.h"
#include "minecraft/network/packet/AddMobPacket.h"
#include "minecraft/network/packet/AddPaintingPacket.h"
#include "minecraft/network/packet/AddPlayerPacket.h"
#include "minecraft/network/packet/JavaTabListPacket.h"
#include "minecraft/network/packet/AnimatePacket.h"
#include "minecraft/network/packet/AwardStatPacket.h"
#include "minecraft/network/packet/BlockRegionUpdatePacket.h"
#include "minecraft/network/packet/ChatPacket.h"
#include "minecraft/network/packet/ChunkTilesUpdatePacket.h"
#include "minecraft/network/packet/ChunkVisibilityAreaPacket.h"
#include "minecraft/network/packet/ChunkVisibilityPacket.h"
#include "minecraft/network/packet/ComplexItemDataPacket.h"
#include "minecraft/network/packet/ContainerAckPacket.h"
#include "minecraft/network/packet/ContainerClosePacket.h"
#include "minecraft/network/packet/ContainerOpenPacket.h"
#include "minecraft/network/packet/ContainerSetContentPacket.h"
#include "minecraft/network/packet/ContainerSetDataPacket.h"
#include "minecraft/network/packet/ContainerSetSlotPacket.h"
#include "minecraft/network/packet/CustomPayloadPacket.h"
#include "minecraft/network/packet/EntityActionAtPositionPacket.h"
#include "minecraft/network/packet/EntityEventPacket.h"
#include "minecraft/network/packet/ExplodePacket.h"
#include "minecraft/network/packet/GameEventPacket.h"
#include "minecraft/network/packet/KeepAlivePacket.h"
#include "minecraft/network/packet/LevelEventPacket.h"
#include "minecraft/network/packet/LevelParticlesPacket.h"
#include "minecraft/network/packet/LevelSoundPacket.h"
#include "minecraft/network/packet/LoginPacket.h"
#include "minecraft/network/packet/MoveEntityPacket.h"
#include "minecraft/network/packet/MoveEntityPacketSmall.h"
#include "minecraft/network/packet/MovePlayerPacket.h"
#include "minecraft/network/packet/PlayerAbilitiesPacket.h"
#include "minecraft/network/packet/PlayerInfoPacket.h"
#include "minecraft/network/packet/PreLoginPacket.h"
#include "minecraft/network/packet/RemoveEntitiesPacket.h"
#include "minecraft/network/packet/RemoveMobEffectPacket.h"
#include "minecraft/network/packet/RespawnPacket.h"
#include "minecraft/network/packet/RotateHeadPacket.h"
#include "minecraft/network/packet/ServerSettingsChangedPacket.h"
#include "minecraft/network/packet/SetCarriedItemPacket.h"
#include "minecraft/network/packet/SetEntityDataPacket.h"
#include "minecraft/network/packet/SetEntityLinkPacket.h"
#include "minecraft/network/packet/SetEntityMotionPacket.h"
#include "minecraft/network/packet/SetEquippedItemPacket.h"
#include "minecraft/network/packet/SetExperiencePacket.h"
#include "minecraft/network/packet/SetHealthPacket.h"
#include "minecraft/network/packet/SetSpawnPositionPacket.h"
#include "minecraft/network/packet/SetTimePacket.h"
#include "minecraft/network/packet/SignUpdatePacket.h"
#include "minecraft/network/packet/TakeItemEntityPacket.h"
#include "minecraft/network/packet/TeleportEntityPacket.h"
#include "minecraft/network/packet/TextureAndGeometryChangePacket.h"
#include "minecraft/network/packet/TextureAndGeometryPacket.h"
#include "minecraft/network/packet/TextureChangePacket.h"
#include "minecraft/network/packet/TexturePacket.h"
#include "minecraft/network/packet/TileDestructionPacket.h"
#include "minecraft/network/packet/TileEditorOpenPacket.h"
#include "minecraft/network/packet/TileEntityDataPacket.h"
#include "minecraft/network/packet/TileEventPacket.h"
#include "minecraft/network/packet/TileUpdatePacket.h"
#include "minecraft/network/packet/UpdateAttributesPacket.h"
#include "minecraft/network/packet/UpdateGameRuleProgressPacket.h"
#include "minecraft/network/packet/UpdateMobEffectPacket.h"
#include "minecraft/network/packet/UpdateProgressPacket.h"
#include "minecraft/network/packet/XZPacket.h"
#include "minecraft/server/MinecraftServer.h"
#include "minecraft/sounds/SoundTypes.h"
#include "minecraft/stats/GenericStats.h"
#include "minecraft/world/SimpleContainer.h"
#include "minecraft/world/effect/MobEffectInstance.h"
#include "minecraft/world/entity/EntityIO.h"
#include "minecraft/world/entity/ExperienceOrb.h"
#include "minecraft/world/entity/ItemFrame.h"
#include "minecraft/world/entity/LeashFenceKnotEntity.h"
#include "minecraft/world/entity/LivingEntity.h"
#include "minecraft/world/entity/Mob.h"
#include "minecraft/world/entity/Painting.h"
#include "minecraft/world/entity/SyncedEntityData.h"
#include "minecraft/world/entity/ai/attributes/AttributeInstance.h"
#include "minecraft/world/entity/ai/attributes/AttributeModifier.h"
#include "minecraft/world/entity/ai/attributes/BaseAttributeMap.h"
#include "minecraft/world/entity/ai/attributes/RangedAttribute.h"
#include "minecraft/world/entity/animal/EntityHorse.h"
#include "minecraft/world/entity/boss/enderdragon/EnderCrystal.h"
#include "minecraft/world/entity/global/LightningBolt.h"
#include "minecraft/world/entity/item/Boat.h"
#include "minecraft/world/entity/item/FallingTile.h"
#include "minecraft/world/entity/item/ItemEntity.h"
#include "minecraft/world/entity/item/Minecart.h"
#include "minecraft/world/entity/item/PrimedTnt.h"
#include "minecraft/world/entity/monster/Slime.h"
#include "minecraft/world/entity/npc/ClientSideMerchant.h"
#include "minecraft/world/entity/player/Abilities.h"
#include "minecraft/world/entity/player/Inventory.h"
#include "minecraft/world/entity/player/Player.h"
#include "minecraft/world/entity/projectile/Arrow.h"
#include "minecraft/world/entity/projectile/DragonFireball.h"
#include "minecraft/world/entity/projectile/EyeOfEnderSignal.h"
#include "minecraft/world/entity/projectile/FireworksRocketEntity.h"
#include "minecraft/world/entity/projectile/FishingHook.h"
#include "minecraft/world/entity/projectile/LargeFireball.h"
#include "minecraft/world/entity/projectile/SmallFireball.h"
#include "minecraft/world/entity/projectile/Snowball.h"
#include "minecraft/world/entity/projectile/ThrownEgg.h"
#include "minecraft/world/entity/projectile/ThrownEnderpearl.h"
#include "minecraft/world/entity/projectile/ThrownExpBottle.h"
#include "minecraft/world/entity/projectile/ThrownPotion.h"
#include "minecraft/world/entity/projectile/WitherSkull.h"
#include "minecraft/world/food/FoodConstants.h"
#include "minecraft/world/food/FoodData.h"
#include "minecraft/world/inventory/AbstractContainerMenu.h"
#include "minecraft/world/inventory/AnimalChest.h"
#include "minecraft/world/inventory/Slot.h"
#include "minecraft/world/item/Item.h"
#include "minecraft/world/item/ItemInstance.h"
#include "minecraft/world/item/MapItem.h"
#include "minecraft/world/item/trading/Merchant.h"
#include "minecraft/world/item/trading/MerchantRecipeList.h"
#include "minecraft/network/packet/HudOverlayPacket.h"
#include "minecraft/network/packet/SetDisplayObjectivePacket.h"
#include "minecraft/network/packet/SetObjectivePacket.h"
#include "minecraft/network/packet/SetPlayerTeamPacket.h"
#include "minecraft/network/packet/SetScorePacket.h"
#include "minecraft/world/level/Explosion.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/scores/Objective.h"
#include "minecraft/world/scores/PlayerTeam.h"
#include "minecraft/world/scores/Score.h"
#include "minecraft/world/scores/Scoreboard.h"
#include "minecraft/world/level/LevelSettings.h"
#include "minecraft/world/level/ViewDistanceUtil.h"
#include "minecraft/world/level/chunk/LevelChunk.h"
#include "minecraft/world/level/dimension/Dimension.h"
#include "minecraft/world/level/saveddata/MapItemSavedData.h"
#include "minecraft/world/level/storage/LevelData.h"
#include "minecraft/world/level/storage/SavedDataStorage.h"
#include "minecraft/world/level/tile/LevelEvent.h"
#include "minecraft/world/level/tile/Tile.h"
#include "minecraft/world/level/tile/entity/BeaconTileEntity.h"
#include "minecraft/world/level/tile/entity/BrewingStandTileEntity.h"
#include "minecraft/world/level/tile/entity/CommandBlockEntity.h"
#include "minecraft/world/level/tile/entity/DispenserTileEntity.h"
#include "minecraft/world/level/tile/entity/DropperTileEntity.h"
#include "minecraft/world/level/tile/entity/FurnaceTileEntity.h"
#include "minecraft/world/level/tile/entity/HopperTileEntity.h"
#include "minecraft/world/level/tile/entity/MobSpawnerTileEntity.h"
#include "minecraft/world/level/tile/entity/SignTileEntity.h"
#include "minecraft/world/level/tile/entity/SkullTileEntity.h"
#include "minecraft/world/level/tile/entity/TileEntity.h"
#include "minecraft/world/phys/AABB.h"
#include "strings.h"

class Packet;
class TexturePack;
class UIScene;

ClientConnection::ClientConnection(Minecraft* minecraft, const std::wstring& ip,
                                   int port) {
    
    assert(false);
}

ClientConnection::ClientConnection(Minecraft* minecraft, Socket* socket,
                                   int iUserIndex ) {
    
    random = new Random();
    done = false;
    level = nullptr;
    started = false;
    savedDataStorage = new SavedDataStorage(nullptr);
    maxPlayers = 20;

    this->minecraft = minecraft;

    if (iUserIndex < 0) {
        m_userIndex = InputManager.GetPrimaryPad();
    } else {
        m_userIndex = iUserIndex;
    }

    if (socket == nullptr) {
        socket = new Socket();  
    }

    createdOk = socket->createdOk;
    if (createdOk) {
        connection = new Connection(socket, L"Client", this);
    } else {
        connection = nullptr;
        
        
        
    }

    deferredEntityLinkPackets = std::vector<DeferredEntityLinkPacket>();
}

ClientConnection::~ClientConnection() {
    delete connection;
    delete random;
    delete savedDataStorage;
}

void ClientConnection::tick() {
    if (connection == nullptr) return;
    if (!done) connection->tick();
    connection->flush();
}

INetworkPlayer* ClientConnection::getNetworkPlayer() {
    if (connection != nullptr && connection->getSocket() != nullptr)
        return connection->getSocket()->getPlayer();
    else
        return nullptr;
}

void ClientConnection::handleLogin(std::shared_ptr<LoginPacket> packet) {
    if (done) return;

    PlayerUID OnlineXuid;
    ProfileManager.GetXUID(m_userIndex, &OnlineXuid, true);  
    MOJANG_DATA* pMojangData = nullptr;

    if (!g_NetworkManager.IsLocalGame()) {
        pMojangData = app.GetMojangDataForXuid(OnlineXuid);
    }

    if (!g_NetworkManager.IsHost()) {
        Minecraft::GetInstance()->progressRenderer->progressStagePercentage(
            (eCCLoginReceived * 100) / (eCCConnected));
    }

    
    
    
    INetworkPlayer* networkPlayer = connection->getSocket()->getPlayer();
    int iUserID = -1;

    if (m_userIndex == InputManager.GetPrimaryPad()) {
        iUserID = m_userIndex;
    } else {
        if (!networkPlayer->IsGuest() && networkPlayer->IsLocal()) {
            
            for (int i = 0; i < XUSER_MAX_COUNT; i++) {
                INetworkPlayer* networkLocalPlayer =
                    g_NetworkManager.GetLocalPlayerByUserIndex(i);
                if (networkLocalPlayer == networkPlayer) {
                    iUserID = i;
                }
            }
        }
    }

    if (iUserID != -1) {
        std::uint8_t* pBuffer = nullptr;
        unsigned int dwSize = 0;
        bool bRes;

        
        if (pMojangData) {
            
            if (pMojangData->wchSkin[0] != 0L) {
                std::wstring wstr = pMojangData->wchSkin;
                
                bRes = app.IsFileInMemoryTextures(wstr);
                if (!bRes) {
                }

                if (bRes) {
                    app.AddMemoryTextureFile(wstr, pBuffer, dwSize);
                }
            }

            
            if (pMojangData->wchCape[0] != 0L) {
                std::wstring wstr = pMojangData->wchCape;
                
                bRes = app.IsFileInMemoryTextures(wstr);
                if (!bRes) {
                }

                if (bRes) {
                    app.AddMemoryTextureFile(wstr, pBuffer, dwSize);
                }
            }
        }

        
        app.ReadBannedList(iUserID);
        
        
        app.SetBanListCheck(iUserID, false);
    }

    if (m_userIndex == InputManager.GetPrimaryPad()) {
        if (app.GetTutorialMode()) {
            minecraft->gameMode = new FullTutorialMode(
                InputManager.GetPrimaryPad(), minecraft, this);
        } else {
            minecraft->gameMode = new ConsoleGameMode(
                InputManager.GetPrimaryPad(), minecraft, this);
        }

        Level* dimensionLevel = minecraft->getLevel(packet->dimension);
        if (dimensionLevel == nullptr) {
            level = new MultiPlayerLevel(
                this,
                new LevelSettings(
                    packet->seed, GameType::byId(packet->gameType), false,
                    false, packet->m_newSeaLevel, packet->m_pLevelType,
                    packet->m_xzSize, packet->m_hellScale),
                packet->dimension, packet->difficulty);

            
            int otherDimensionId = packet->dimension == 0 ? -1 : 0;
            Level* activeLevel = minecraft->getLevel(otherDimensionId);
            if (activeLevel != nullptr) {
                
                
                
                
                level->savedDataStorage = activeLevel->savedDataStorage;
            }

            app.DebugPrintf("ClientConnection - DIFFICULTY --- %d\n",
                            packet->difficulty);
            level->difficulty = packet->difficulty;  
            level->isClientSide = true;

            if (!g_NetworkManager.IsHost()) {
                int primaryPad = InputManager.GetPrimaryPad();
                if (minecraft->player != nullptr) {
                    fprintf(stderr,
                            "[TCP] Clearing stale minecraft->player before "
                            "setLevel(MP) to force fresh creation\n");
                    minecraft->player = nullptr;
                }
                if (minecraft->localplayers[primaryPad] != nullptr) {
                    minecraft->localplayers[primaryPad] = nullptr;
                }
                minecraft->cameraTargetPlayer = nullptr;
            }

            minecraft->setLevel(level);
        }

        minecraft->player->setPlayerIndex(packet->m_playerIndex);
        minecraft->player->setCustomSkin(app.GetPlayerSkinId(m_userIndex));
        minecraft->player->setCustomCape(app.GetPlayerCapeId(m_userIndex));
        minecraft->createPrimaryLocalPlayer(InputManager.GetPrimaryPad());
        ((MultiPlayerGameMode*)minecraft->gameMode)
            ->setLocalMode(GameType::byId(packet->gameType));
        fprintf(stderr,
                "[TCP] Set primary local gameMode to id=%d for remote "
                "client\n",
                (int)packet->gameType);

        if (!g_NetworkManager.IsHost()) {
            int primaryPad = InputManager.GetPrimaryPad();
            if (minecraft->localplayers[primaryPad] != nullptr) {
                minecraft->localplayers[primaryPad]->connection = this;
                fprintf(stderr,
                        "[TCP] Bound ClientConnection to "
                        "localplayers[%d]->connection for remote client\n",
                        primaryPad);
            }
            if (minecraft->levelRenderer != nullptr && level != nullptr) {
                minecraft->levelRenderer->setLevel(primaryPad, level);
                if (minecraft->particleEngine != nullptr) {
                    minecraft->particleEngine->setLevel(level);
                }
                fprintf(stderr,
                        "[TCP] Bound LevelRenderer[pad=%d] to level=%p\n",
                        primaryPad, (void*)level);
            }
            
            
            
            minecraft->cameraTargetPlayer = minecraft->player;
        }

        {
            int clientOptionChunks =
                viewDistanceOptionToChunks(minecraft->options->viewDistance);
            int effective;
            const bool isLocalOrHost =
                g_NetworkManager.IsHost() ||
                (connection != nullptr && connection->getSocket() != nullptr &&
                 connection->getSocket()->isLocal());
            if (isLocalOrHost) {
                effective = clampViewDistance(packet->serverViewDistance);
            } else {
                effective = effectiveViewDistance(packet->serverViewDistance,
                                                  clientOptionChunks);
            }
            minecraft->m_serverViewDistanceChunks = effective;
            app.DebugPrintf(
                "ClientConnection - Effective_View_Distance = %d chunks "
                "(serverVD=%d, clientOption=%d->%d chunks, %s)\n",
                effective, packet->serverViewDistance,
                minecraft->options->viewDistance, clientOptionChunks,
                isLocalOrHost ? "local/host" : "remote");
        }

        minecraft->player->entityId = packet->clientVersion;
        minecraft->player->dimension = packet->dimension;
        minecraft->setScreen(new ReceivingLevelScreen(this));

        std::uint8_t networkSmallId = getSocket()->getSmallId();
        app.UpdatePlayerInfo(networkSmallId, packet->m_playerIndex,
                             packet->m_uiGamePrivileges);
        minecraft->player->setPlayerGamePrivilege(
            Player::ePlayerGamePrivilege_All, packet->m_uiGamePrivileges);

        
        
        unsigned int startingPrivileges = 0;
        Player::enableAllPlayerPrivileges(startingPrivileges, true);

        if (networkPlayer->IsHost()) {
            Player::setPlayerGamePrivilege(
                startingPrivileges, Player::ePlayerGamePrivilege_HOST, 1);
        }

        if (!g_NetworkManager.IsHost()) {
            startingPrivileges = packet->m_uiGamePrivileges;
        }

        displayPrivilegeChanges(minecraft->player, startingPrivileges);

        
        app.SetGameSettingsDebugMask(InputManager.GetPrimaryPad(),
                                     app.GetGameSettingsDebugMask(-1, true));
    } else {
        
        
        level = (MultiPlayerLevel*)minecraft->getLevel(packet->dimension);
        std::shared_ptr<Player> player;

        if (level == nullptr) {
            int otherDimensionId = packet->dimension == 0 ? -1 : 0;
            MultiPlayerLevel* activeLevel =
                minecraft->getLevel(otherDimensionId);

            if (activeLevel == nullptr) {
                otherDimensionId = packet->dimension == 0
                                       ? 1
                                       : (packet->dimension == -1 ? 1 : -1);
                activeLevel = minecraft->getLevel(otherDimensionId);
            }

            MultiPlayerLevel* dimensionLevel = new MultiPlayerLevel(
                this,
                new LevelSettings(
                    packet->seed, GameType::byId(packet->gameType), false,
                    false, packet->m_newSeaLevel, packet->m_pLevelType,
                    packet->m_xzSize, packet->m_hellScale),
                packet->dimension, packet->difficulty);

            dimensionLevel->savedDataStorage = activeLevel->savedDataStorage;

            dimensionLevel->difficulty = packet->difficulty;  
            dimensionLevel->isClientSide = true;
            level = dimensionLevel;
            
            
            
            
            MultiPlayerLevel* levelpassedin = (MultiPlayerLevel*)level;
            player = minecraft->createExtraLocalPlayer(
                m_userIndex, networkPlayer->GetOnlineName(), m_userIndex,
                packet->dimension, this, levelpassedin);

            
            std::shared_ptr<MultiplayerLocalPlayer> lastPlayer =
                minecraft->player;
            minecraft->player = minecraft->localplayers[m_userIndex];
            minecraft->setLevel(level);
            minecraft->player = lastPlayer;
        } else {
            player = minecraft->createExtraLocalPlayer(
                m_userIndex, networkPlayer->GetOnlineName(), m_userIndex,
                packet->dimension, this);
        }

        
        player->dimension = packet->dimension;
        player->entityId = packet->clientVersion;

        player->setPlayerIndex(packet->m_playerIndex);
        player->setCustomSkin(app.GetPlayerSkinId(m_userIndex));
        player->setCustomCape(app.GetPlayerCapeId(m_userIndex));

        std::uint8_t networkSmallId = getSocket()->getSmallId();
        app.UpdatePlayerInfo(networkSmallId, packet->m_playerIndex,
                             packet->m_uiGamePrivileges);
        player->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_All,
                                       packet->m_uiGamePrivileges);

        
        
        unsigned int startingPrivileges = 0;
        Player::enableAllPlayerPrivileges(startingPrivileges, true);

        displayPrivilegeChanges(minecraft->localplayers[m_userIndex],
                                startingPrivileges);
    }

    maxPlayers = packet->maxPlayers;

    
    std::shared_ptr<MultiplayerLocalPlayer> lastPlayer = minecraft->player;
    minecraft->player = minecraft->localplayers[m_userIndex];
    ((MultiPlayerGameMode*)minecraft->localgameModes[m_userIndex])
        ->setLocalMode(GameType::byId(packet->gameType));
    minecraft->player = lastPlayer;

    
    if (iUserID != -1) {
        ui.UpdateSelectedItemPos(iUserID);
    }
}

void ClientConnection::handleAddEntity(
    std::shared_ptr<AddEntityPacket> packet) {
    if (level == nullptr) {
        fprintf(stderr,
                "[TCP] handleAddEntity dropped: level not ready yet "
                "(id=%d type=%d)\n",
                packet->id, packet->type);
        return;
    }
    double x = packet->x / 32.0;
    double y = packet->y / 32.0;
    double z = packet->z / 32.0;
    std::shared_ptr<Entity> e;
    bool setRot = true;

    
    switch (packet->type) {
        case AddEntityPacket::MINECART:
            e = Minecart::createMinecart(level, x, y, z, packet->data);
            break;
        case AddEntityPacket::FISH_HOOK: {
            
            
            std::shared_ptr<Entity> owner = getEntity(packet->data);

            
            if (owner == nullptr) {
                for (int i = 0; i < XUSER_MAX_COUNT; i++) {
                    if (minecraft->localplayers[i]) {
                        if (minecraft->localplayers[i]->entityId ==
                            packet->data) {
                            owner = minecraft->localplayers[i];
                            break;
                        }
                    }
                }
            }

            if (owner->instanceof(eTYPE_PLAYER)) {
                std::shared_ptr<Player> player =
                    std::dynamic_pointer_cast<Player>(owner);
                std::shared_ptr<FishingHook> hook =
                    std::shared_ptr<FishingHook>(
                        new FishingHook(level, x, y, z, player));
                e = hook;
                
                
                player->fishing = hook;
            }
            packet->data = 0;
        } break;
        case AddEntityPacket::ARROW:
            e = std::make_shared<Arrow>(level, x, y, z);
            break;
        case AddEntityPacket::SNOWBALL:
            e = std::make_shared<Snowball>(level, x, y, z);
            break;
        case AddEntityPacket::ITEM_FRAME: {
            int ix = (int)x;
            int iy = (int)y;
            int iz = (int)z;
            app.DebugPrintf("ClientConnection ITEM_FRAME xyz %d,%d,%d\n", ix,
                            iy, iz);
        }
            e = std::shared_ptr<Entity>(
                new ItemFrame(level, (int)x, (int)y, (int)z, packet->data));
            packet->data = 0;
            setRot = false;
            break;
        case AddEntityPacket::THROWN_ENDERPEARL:
            e = std::make_shared<ThrownEnderpearl>(level, x, y, z);
            break;
        case AddEntityPacket::EYEOFENDERSIGNAL:
            e = std::make_shared<EyeOfEnderSignal>(level, x, y, z);
            break;
        case AddEntityPacket::FIREBALL:
            e = std::shared_ptr<Entity>(
                new LargeFireball(level, x, y, z, packet->xa / 8000.0,
                                  packet->ya / 8000.0, packet->za / 8000.0));
            packet->data = 0;
            break;
        case AddEntityPacket::SMALL_FIREBALL:
            e = std::shared_ptr<Entity>(
                new SmallFireball(level, x, y, z, packet->xa / 8000.0,
                                  packet->ya / 8000.0, packet->za / 8000.0));
            packet->data = 0;
            break;
        case AddEntityPacket::DRAGON_FIRE_BALL:
            e = std::shared_ptr<Entity>(
                new DragonFireball(level, x, y, z, packet->xa / 8000.0,
                                   packet->ya / 8000.0, packet->za / 8000.0));
            packet->data = 0;
            break;
        case AddEntityPacket::EGG:
            e = std::make_shared<ThrownEgg>(level, x, y, z);
            break;
        case AddEntityPacket::THROWN_POTION:
            e = std::shared_ptr<Entity>(
                new ThrownPotion(level, x, y, z, packet->data));
            packet->data = 0;
            break;
        case AddEntityPacket::THROWN_EXPBOTTLE:
            e = std::make_shared<ThrownExpBottle>(level, x, y, z);
            packet->data = 0;
            break;
        case AddEntityPacket::BOAT:
            e = std::make_shared<Boat>(level, x, y, z);
            break;
        case AddEntityPacket::PRIMED_TNT:
            e = std::make_shared<PrimedTnt>(level, x, y, z, nullptr);
            break;
        case AddEntityPacket::ENDER_CRYSTAL:
            e = std::make_shared<EnderCrystal>(level, x, y, z);
            break;
        case AddEntityPacket::ITEM:
            e = std::make_shared<ItemEntity>(level, x, y, z);
            break;
        case AddEntityPacket::FALLING:
            e = std::make_shared<FallingTile>(
                level, x, y, z, packet->data & 0xFFFF, packet->data >> 16);
            packet->data = 0;
            break;
        case AddEntityPacket::WITHER_SKULL:
            e = std::shared_ptr<Entity>(
                new WitherSkull(level, x, y, z, packet->xa / 8000.0,
                                packet->ya / 8000.0, packet->za / 8000.0));
            packet->data = 0;
            break;
        case AddEntityPacket::FIREWORKS:
            e = std::shared_ptr<Entity>(
                new FireworksRocketEntity(level, x, y, z, nullptr));
            break;
        case AddEntityPacket::LEASH_KNOT:
            e = std::shared_ptr<Entity>(
                new LeashFenceKnotEntity(level, (int)x, (int)y, (int)z));
            packet->data = 0;
            break;
        default:
            fprintf(stderr, "[TCP] handleAddEntity: unsupported type %d "
                            "(id=%d) - dropped\n",
                    packet->type, packet->id);
            break;
    }

    





















































































    if (e != nullptr) {
        e->xp = packet->x;
        e->yp = packet->y;
        e->zp = packet->z;

        float yRot = packet->yRot * 360 / 256.0f;
        float xRot = packet->xRot * 360 / 256.0f;
        e->yRotp = packet->yRot;
        e->xRotp = packet->xRot;

        if (setRot) {
            e->yRot = 0.0f;
            e->xRot = 0.0f;
        }

        std::vector<std::shared_ptr<Entity> >* subEntities =
            e->getSubEntities();
        if (subEntities != nullptr) {
            int offs = packet->id - e->entityId;
            
            for (auto it = subEntities->begin(); it != subEntities->end();
                 ++it) {
                (*it)->entityId += offs;
                
                
            }
        }

        if (packet->type == AddEntityPacket::LEASH_KNOT) {
            
            
            e->absMoveTo(e->x, e->y, e->z, yRot, xRot);
        } else if (packet->type == AddEntityPacket::ITEM_FRAME) {
            
            
            
            
        } else {
            
            e->absMoveTo(x, y, z, yRot, xRot);
        }
        e->entityId = packet->id;
        level->putEntity(packet->id, e);

        if (packet->data > -1)  
                                
        {
            if (packet->type == AddEntityPacket::ARROW) {
                std::shared_ptr<Entity> owner = getEntity(packet->data);

                
                if (owner == nullptr) {
                    for (int i = 0; i < XUSER_MAX_COUNT; i++) {
                        if (minecraft->localplayers[i]) {
                            if (minecraft->localplayers[i]->entityId ==
                                packet->data) {
                                owner = minecraft->localplayers[i];
                                break;
                            }
                        }
                    }
                }

                if (owner != nullptr && owner->instanceof(eTYPE_LIVINGENTITY)) {
                    std::dynamic_pointer_cast<Arrow>(e)->owner =
                        std::dynamic_pointer_cast<LivingEntity>(owner);
                }
            }

            e->lerpMotion(packet->xa / 8000.0, packet->ya / 8000.0,
                          packet->za / 8000.0);
        }

        
        checkDeferredEntityLinkPackets(e->entityId);
    }
}

void ClientConnection::handleAddExperienceOrb(
    std::shared_ptr<AddExperienceOrbPacket> packet) {
    if (level == nullptr) return;
    std::shared_ptr<Entity> e = std::shared_ptr<ExperienceOrb>(
        new ExperienceOrb(level, packet->x / 32.0, packet->y / 32.0,
                          packet->z / 32.0, packet->value));
    e->xp = packet->x;
    e->yp = packet->y;
    e->zp = packet->z;
    e->yRot = 0;
    e->xRot = 0;
    e->entityId = packet->id;
    level->putEntity(packet->id, e);
}

void ClientConnection::handleAddGlobalEntity(
    std::shared_ptr<AddGlobalEntityPacket> packet) {
    if (level == nullptr) return;
    double x = packet->x / 32.0;
    double y = packet->y / 32.0;
    double z = packet->z / 32.0;
    std::shared_ptr<Entity> e;  
    if (packet->type == AddGlobalEntityPacket::LIGHTNING)
        e = std::make_shared<LightningBolt>(level, x, y, z);
    if (e != nullptr) {
        e->xp = packet->x;
        e->yp = packet->y;
        e->zp = packet->z;
        e->yRot = 0;
        e->xRot = 0;
        e->entityId = packet->id;
        level->addGlobalEntity(e);
    }
}

void ClientConnection::handleAddPainting(
    std::shared_ptr<AddPaintingPacket> packet) {
    if (level == nullptr) return;
    std::shared_ptr<Painting> painting = std::make_shared<Painting>(
        level, packet->x, packet->y, packet->z, packet->dir, packet->motive);
    level->putEntity(packet->id, painting);
}

void ClientConnection::handleSetEntityMotion(
    std::shared_ptr<SetEntityMotionPacket> packet) {
    std::shared_ptr<Entity> e = getEntity(packet->id);
    if (e == nullptr) return;
    e->lerpMotion(packet->xa / 8000.0, packet->ya / 8000.0,
                  packet->za / 8000.0);
}

void ClientConnection::handleSetEntityData(
    std::shared_ptr<SetEntityDataPacket> packet) {
    std::shared_ptr<Entity> e = getEntity(packet->id);
    if (e != nullptr && packet->getUnpackedData() != nullptr) {
        e->getEntityData()->assignValues(packet->getUnpackedData());
    }
}

void ClientConnection::handleAddPlayer(
    std::shared_ptr<AddPlayerPacket> packet) {
    if (level == nullptr) {
        fprintf(stderr,
                "[TCP] handleAddPlayer dropped: ClientConnection has no "
                "level yet (id=%d name=%ls)\n",
                packet->id, packet->name.c_str());
        return;
    }
    
    
    for (unsigned int idx = 0; idx < XUSER_MAX_COUNT; ++idx) {
        
        PlayerUID playerXUIDOnline = INVALID_XUID,
                  playerXUIDOffline = INVALID_XUID;
        ProfileManager.GetXUID(idx, &playerXUIDOnline, true);
        ProfileManager.GetXUID(idx, &playerXUIDOffline, false);
        if ((playerXUIDOnline != INVALID_XUID &&
             ProfileManager.AreXUIDSEqual(playerXUIDOnline, packet->xuid)) ||
            (playerXUIDOffline != INVALID_XUID &&
             ProfileManager.AreXUIDSEqual(playerXUIDOffline, packet->xuid))) {
            app.DebugPrintf(
                "AddPlayerPacket received with XUID of local player\n");
            return;
        }
    }
    double x = packet->x / 32.0;
    double y = packet->y / 32.0;
    double z = packet->z / 32.0;
    float yRot = packet->yRot * 360 / 256.0f;
    float xRot = packet->xRot * 360 / 256.0f;
    std::shared_ptr<RemotePlayer> player = std::shared_ptr<RemotePlayer>(
        new RemotePlayer(level, packet->name));
    player->xo = player->xOld = player->xp = packet->x;
    player->yo = player->yOld = player->yp = packet->y;
    player->zo = player->zOld = player->zp = packet->z;
    player->xRotp = packet->xRot;
    player->yRotp = packet->yRot;
    player->yHeadRot = packet->yHeadRot * 360 / 256.0f;
    player->setXuid(packet->xuid);

    
    
    player->m_displayName = player->name;

    

    int item = packet->carriedItem;
    if (item == 0) {
        player->inventory->items[player->inventory->selected] =
            std::shared_ptr<ItemInstance>();  
    } else {
        player->inventory->items[player->inventory->selected] =
            std::make_shared<ItemInstance>(item, 1, 0);
    }
    player->absMoveTo(x, y, z, yRot, xRot);

    player->setPlayerIndex(packet->m_playerIndex);
    player->setCustomSkin(packet->m_skinId);
    player->setCustomCape(packet->m_capeId);
    player->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_All,
                                   packet->m_uiGamePrivileges);

    if (!player->customTextureUrl.empty() &&
        player->customTextureUrl.substr(0, 3).compare(L"def") != 0 &&
        !app.IsFileInMemoryTextures(player->customTextureUrl)) {
        if (minecraft->addPendingClientTextureRequest(
                player->customTextureUrl)) {
            app.DebugPrintf(
                "Client sending TextureAndGeometryPacket to get custom skin "
                "%ls for player %ls\n",
                player->customTextureUrl.c_str(), player->name.c_str());

            send(std::shared_ptr<TextureAndGeometryPacket>(
                new TextureAndGeometryPacket(player->customTextureUrl, nullptr,
                                             0)));
        }
    } else if (!player->customTextureUrl.empty() &&
               app.IsFileInMemoryTextures(player->customTextureUrl)) {
        
        app.AddMemoryTextureFile(player->customTextureUrl, nullptr, 0);
    }

    app.DebugPrintf("Custom skin for player %ls is %ls\n", player->name.c_str(),
                    player->customTextureUrl.c_str());

    if (!player->customTextureUrl2.empty() &&
        player->customTextureUrl2.substr(0, 3).compare(L"def") != 0 &&
        !app.IsFileInMemoryTextures(player->customTextureUrl2)) {
        if (minecraft->addPendingClientTextureRequest(
                player->customTextureUrl2)) {
            app.DebugPrintf(
                "Client sending texture packet to get custom cape %ls for "
                "player %ls\n",
                player->customTextureUrl2.c_str(), player->name.c_str());
            send(std::shared_ptr<TexturePacket>(
                new TexturePacket(player->customTextureUrl2, nullptr, 0)));
        }
    } else if (!player->customTextureUrl2.empty() &&
               app.IsFileInMemoryTextures(player->customTextureUrl2)) {
        
        app.AddMemoryTextureFile(player->customTextureUrl2, nullptr, 0);
    }

    app.DebugPrintf("Custom cape for player %ls is %ls\n", player->name.c_str(),
                    player->customTextureUrl2.c_str());

    level->putEntity(packet->id, player);

    std::vector<std::shared_ptr<SynchedEntityData::DataItem> >* unpackedData =
        packet->getUnpackedData();
    if (unpackedData != nullptr) {
        player->getEntityData()->assignValues(unpackedData);
    }
}

void ClientConnection::handleTeleportEntity(
    std::shared_ptr<TeleportEntityPacket> packet) {
    std::shared_ptr<Entity> e = getEntity(packet->id);
    if (e == nullptr) return;
    e->xp = packet->x;
    e->yp = packet->y;
    e->zp = packet->z;
    double x = e->xp / 32.0;
    double y = e->yp / 32.0 + 1 / 64.0f;
    double z = e->zp / 32.0;
    
    int ixRot = packet->xRot;
    if (ixRot >= 128) ixRot -= 256;
    float yRot = packet->yRot * 360 / 256.0f;
    float xRot = ixRot * 360 / 256.0f;
    e->yRotp = packet->yRot;
    e->xRotp = ixRot;

    
    
    e->lerpTo(x, y, z, yRot, xRot, 3);
}

void ClientConnection::handleSetCarriedItem(
    std::shared_ptr<SetCarriedItemPacket> packet) {
    if (packet->slot >= 0 && packet->slot < Inventory::getSelectionSize()) {
        Minecraft::GetInstance()
            ->localplayers[m_userIndex]
            .get()
            ->inventory->selected = packet->slot;
    }
}

void ClientConnection::handleMoveEntity(
    std::shared_ptr<MoveEntityPacket> packet) {
    std::shared_ptr<Entity> e = getEntity(packet->id);
    if (e == nullptr) return;
    e->xp += packet->xa;
    e->yp += packet->ya;
    e->zp += packet->za;
    double x = e->xp / 32.0;
    
    
    double y = e->yp / 32.0 + 1 / 64.0f;
    double z = e->zp / 32.0;
    
    e->yRotp += packet->yRot;
    e->xRotp += packet->xRot;
    float yRot = (e->yRotp * 360) / 256.0f;
    float xRot = (e->xRotp * 360) / 256.0f;
    
    
    e->lerpTo(x, y, z, yRot, xRot, 3);
}

void ClientConnection::handleRotateMob(
    std::shared_ptr<RotateHeadPacket> packet) {
    std::shared_ptr<Entity> e = getEntity(packet->id);
    if (e == nullptr) return;
    float yHeadRot = packet->yHeadRot * 360 / 256.f;
    e->setYHeadRot(yHeadRot);
}

void ClientConnection::handleMoveEntitySmall(
    std::shared_ptr<MoveEntityPacketSmall> packet) {
    std::shared_ptr<Entity> e = getEntity(packet->id);
    if (e == nullptr) return;
    e->xp += packet->xa;
    e->yp += packet->ya;
    e->zp += packet->za;
    double x = e->xp / 32.0;
    
    
    double y = e->yp / 32.0 + 1 / 64.0f;
    double z = e->zp / 32.0;
    
    e->yRotp += packet->yRot;
    e->xRotp += packet->xRot;
    float yRot = (e->yRotp * 360) / 256.0f;
    float xRot = (e->xRotp * 360) / 256.0f;
    
    
    e->lerpTo(x, y, z, yRot, xRot, 3);
}

void ClientConnection::handleRemoveEntity(
    std::shared_ptr<RemoveEntitiesPacket> packet) {
    if (level == nullptr) return;
    for (int i = 0; i < packet->ids.size(); i++) {
        level->removeEntity(packet->ids[i]);
    }
}

void ClientConnection::handleMovePlayer(
    std::shared_ptr<MovePlayerPacket> packet) {
    std::shared_ptr<Player> player =
        minecraft->localplayers[m_userIndex];  

    if (player == nullptr) {
        fprintf(stderr,
                "[TCP] handleMovePlayer skipped: localplayers[%d] not "
                "ready yet\n",
                m_userIndex);
        return;
    }

    double x = player->x;
    double y = player->y;
    double z = player->z;
    float yRot = player->yRot;
    float xRot = player->xRot;

    if (packet->hasPos) {
        x = packet->x;
        y = packet->y;
        z = packet->z;
    }
    if (packet->hasRot) {
        yRot = packet->yRot;
        xRot = packet->xRot;
    }

    player->ySlideOffset = 0;
    player->xd = player->yd = player->zd = 0;
    player->absMoveTo(x, y, z, yRot, xRot);
    packet->x = player->x;
    packet->y = player->bb.y0;
    packet->z = player->z;
    packet->yView = player->y;
    connection->send(packet);
    if (!started) {
        if (!g_NetworkManager.IsHost() && !m_finishingPendingStart) {
            m_pendingStart = true;
            m_pendingStartX = x;
            m_pendingStartY = y;
            m_pendingStartZ = z;
            m_pendingStartYRot = yRot;
            m_pendingStartXRot = xRot;
            tryFinishStartedHandshake();
            if (m_pendingStart) return;
        }

        if (!g_NetworkManager.IsHost()) {
            Minecraft::GetInstance()->progressRenderer->progressStagePercentage(
                (eCCConnected * 100) / (eCCConnected));
        }
        player->xo = player->x;
        player->yo = player->y;
        player->zo = player->z;
        
        
        
        player->xOld = player->x;
        player->yOld = player->y;
        player->zOld = player->z;

        started = true;
        minecraft->setScreen(nullptr);
        if (!g_NetworkManager.IsHost() && !app.GetGameStarted()) {
            app.SetGameStarted(true);
            fprintf(stderr, "[TCP] Forced SetGameStarted(true) on remote "
                            "client\n");
        }

        if (app.GetGameStarted()) {
            ui.CloseUIScenes(m_userIndex);
        }

        if (!g_NetworkManager.IsHost() &&
            minecraft->levelRenderer != nullptr) {
            fprintf(stderr,
                    "[TCP] First MovePlayer for remote client - asking "
                    "LevelRenderer to recentre around (%.1f,%.1f,%.1f)\n",
                    player->x, player->y, player->z);
            minecraft->levelRenderer->invalidateLastPlayerPos(m_userIndex);
        }
    }
}

void ClientConnection::tryFinishStartedHandshake() {
    if (!m_pendingStart || started || m_brupsSinceLogin < 3) return;

    int chunkX = (int)std::floor(m_pendingStartX) >> 4;
    int chunkZ = (int)std::floor(m_pendingStartZ) >> 4;
    if (level == nullptr || !level->reallyHasChunk(chunkX, chunkZ)) return;

    std::shared_ptr<MovePlayerPacket> packet(new MovePlayerPacket::PosRot(
        m_pendingStartX, m_pendingStartY, m_pendingStartY + 1.62,
        m_pendingStartZ, m_pendingStartYRot, m_pendingStartXRot, true, false));
    m_pendingStart = false;
    m_finishingPendingStart = true;
    handleMovePlayer(packet);
    m_finishingPendingStart = false;
}


void ClientConnection::handleChunkVisibilityArea(
    std::shared_ptr<ChunkVisibilityAreaPacket> packet) {
    for (int z = packet->m_minZ; z <= packet->m_maxZ; ++z)
        for (int x = packet->m_minX; x <= packet->m_maxX; ++x)
            level->setChunkVisible(x, z, true);
}

void ClientConnection::handleChunkVisibility(
    std::shared_ptr<ChunkVisibilityPacket> packet) {
    level->setChunkVisible(packet->x, packet->z, packet->visible);
}

void ClientConnection::handleChunkTilesUpdate(
    std::shared_ptr<ChunkTilesUpdatePacket> packet) {
    
    if (packet->levelIdx < 0 ||
        packet->levelIdx >= (int)minecraft->levels.size()) {
        return;
    }
    MultiPlayerLevel* dimensionLevel =
        (MultiPlayerLevel*)minecraft->levels[packet->levelIdx];
    if (dimensionLevel) {
        LevelChunk* lc = dimensionLevel->getChunk(packet->xc, packet->zc);
        int xo = packet->xc * 16;
        int zo = packet->zc * 16;
        
        
        
        
        
        bool forcedUnshare = false;
        for (int i = 0; i < packet->count; i++) {
            int pos = packet->positions[i];
            int tile = packet->blocks[i] & 0xff;
            int data = packet->data[i];

            int x = (pos >> 12) & 15;
            int z = (pos >> 8) & 15;
            int y = ((pos) & 255);

            
            int prevTile = lc->getTile(x, y, z);
            if ((tile != prevTile && !forcedUnshare)) {
                dimensionLevel->unshareChunkAt(xo, zo);

                forcedUnshare = true;
            }

            
            
            
            
            
            
            
            
            
            lc->setTileAndData(x, y, z, tile, data);
            dimensionLevel->checkLight(x + xo, y, z + zo);

            dimensionLevel->clearResetRegion(x + xo, y, z + zo, x + xo, y,
                                             z + zo);

            
            
            
            if (!(((prevTile == Tile::water_Id) &&
                   (tile == Tile::calmWater_Id)) ||
                  ((prevTile == Tile::calmWater_Id) &&
                   (tile == Tile::water_Id)) ||
                  ((prevTile == Tile::lava_Id) &&
                   (tile == Tile::calmLava_Id)) ||
                  ((prevTile == Tile::calmLava_Id) &&
                   (tile == Tile::calmLava_Id)) ||
                  ((prevTile == Tile::calmLava_Id) &&
                   (tile == Tile::lava_Id)))) {
                dimensionLevel->setTilesDirty(x + xo, y, z + zo, x + xo, y,
                                              z + zo);
            }

            
            
            
            
            
            dimensionLevel->removeUnusedTileEntitiesInRegion(
                xo + x, y, zo + z, xo + x + 1, y + 1, zo + z + 1);
        }
        dimensionLevel->shareChunkAt(xo,
                                     zo);  
                                           
    }
}

void ClientConnection::handleBlockRegionUpdate(
    std::shared_ptr<BlockRegionUpdatePacket> packet) {
    
    if (packet->levelIdx < 0 ||
        packet->levelIdx >= (int)minecraft->levels.size()) {
        fprintf(stderr,
                "[TCP] handleBlockRegionUpdate dropped: bad levelIdx=%d "
                "(size=%zu)\n",
                packet->levelIdx, minecraft->levels.size());
        return;
    }
    MultiPlayerLevel* dimensionLevel =
        (MultiPlayerLevel*)minecraft->levels[packet->levelIdx];
    if (dimensionLevel) {
        int y1 = packet->y + packet->ys;
        if (packet->bIsFullChunk) {
            y1 = Level::maxBuildHeight;
            if (packet->buffer.size() > 0) {
                LevelChunk::reorderBlocksAndDataToXZY(packet->y, packet->xs,
                                                      packet->ys, packet->zs,
                                                      &packet->buffer);
            }
        }
        if (!g_NetworkManager.IsHost()) {
            int xc0 = packet->x >> 4;
            int zc0 = packet->z >> 4;
            int xc1 = (packet->x + packet->xs - 1) >> 4;
            int zc1 = (packet->z + packet->zs - 1) >> 4;
            for (int xc = xc0; xc <= xc1; ++xc) {
                for (int zc = zc0; zc <= zc1; ++zc) {
                    dimensionLevel->setChunkVisible(xc, zc, true);
                }
            }
        }
        dimensionLevel->clearResetRegion(packet->x, packet->y, packet->z,
                                         packet->x + packet->xs - 1, y1 - 1,
                                         packet->z + packet->zs - 1);

        
        
        dimensionLevel->setBlocksAndData(packet->x, packet->y, packet->z,
                                         packet->xs, packet->ys, packet->zs,
                                         packet->buffer, packet->bIsFullChunk);

        

        
        
        
        
        
        dimensionLevel->removeUnusedTileEntitiesInRegion(
            packet->x, packet->y, packet->z, packet->x + packet->xs, y1,
            packet->z + packet->zs);

        
        
        
        
        if (packet->bIsFullChunk) {
            int chunkX = packet->x >> 4;
            int chunkZ = packet->z >> 4;
            if (!g_NetworkManager.IsHost()) {
                LevelChunk* lc = dimensionLevel->getChunk(chunkX, chunkZ);
                lc->recalcHeightmap();
                dimensionLevel->setTilesDirty(packet->x, 0, packet->z,
                                              packet->x + 15,
                                              Level::maxBuildHeight - 1,
                                              packet->z + 15);
            }
            dimensionLevel->dataReceivedForChunk(chunkX, chunkZ);
            if (!g_NetworkManager.IsHost()) {
                ++m_brupsSinceLogin;
                tryFinishStartedHandshake();
            }
        }
    }
}

void ClientConnection::handleTileUpdate(
    std::shared_ptr<TileUpdatePacket> packet) {
    
    
    
    
    
    bool destroyTilePacket = false;
    if (packet->block == 255) {
        packet->block = 0;
        destroyTilePacket = true;
    }
    
    if (packet->levelIdx < 0 ||
        packet->levelIdx >= (int)minecraft->levels.size()) {
        return;
    }
    MultiPlayerLevel* dimensionLevel =
        (MultiPlayerLevel*)minecraft->levels[packet->levelIdx];
    if (dimensionLevel) {
        if (g_NetworkManager.IsHost()) {
            
            
            
            
            
            int prevTile =
                dimensionLevel->getTile(packet->x, packet->y, packet->z);
            int prevData =
                dimensionLevel->getData(packet->x, packet->y, packet->z);
            if (packet->block != prevTile || packet->data != prevData) {
                dimensionLevel->unshareChunkAt(packet->x, packet->z);
            }
        }

        
        
        
        
        if (destroyTilePacket) {
            minecraft->levelRenderer->destroyedTileManager->destroyingTileAt(
                dimensionLevel, packet->x, packet->y, packet->z);
        }

        bool tileWasSet = dimensionLevel->doSetTileAndData(
            packet->x, packet->y, packet->z, packet->block, packet->data);

        
        
        
        
        
        dimensionLevel->removeUnusedTileEntitiesInRegion(
            packet->x, packet->y, packet->z, packet->x + 1, packet->y + 1,
            packet->z + 1);

        dimensionLevel->shareChunkAt(
            packet->x, packet->z);  
                                    
    }
}

void ClientConnection::handleDisconnect(
    std::shared_ptr<DisconnectPacket> packet) {
#if defined(__linux__) && defined(__APPLE__)
    
    
    
    
    if (connection && connection->getSocket() &&
        connection->getSocket()->isLocal()) {
        fprintf(stderr,
                "[CONN] Ignoring DisconnectPacket on local connection "
                "(reason=%d)\n",
                packet->reason);
        return;
    }
#endif
    connection->close(DisconnectPacket::eDisconnect_Kicked);
    done = true;

    Minecraft* pMinecraft = Minecraft::GetInstance();
    pMinecraft->connectionDisconnected(m_userIndex, packet->reason);
    app.SetDisconnectReason(packet->reason);
    app.SetDisconnectReasonText(packet->m_customText);
    app.SetAction(m_userIndex, eAppAction_ExitWorld, (void*)true);
    
    
    
}

void ClientConnection::onDisconnect(DisconnectPacket::eDisconnectReason reason,
                                    void* reasonObjects) {
    if (done) return;
    done = true;

    Minecraft* pMinecraft = Minecraft::GetInstance();
    pMinecraft->connectionDisconnected(m_userIndex, reason);

    
    
    
    
    if (g_NetworkManager.IsHost() &&
        (reason == DisconnectPacket::eDisconnect_TimeOut ||
         reason == DisconnectPacket::eDisconnect_Overflow) &&
        m_userIndex == InputManager.GetPrimaryPad() &&
        !MinecraftServer::saveOnExitAnswered()) {
        unsigned int uiIDA[1];
        uiIDA[0] = IDS_CONFIRM_OK;
        ui.RequestErrorMessage(IDS_EXITING_GAME, IDS_GENERIC_ERROR, uiIDA, 1,
                               InputManager.GetPrimaryPad(),
                               &ClientConnection::HostDisconnectReturned,
                               nullptr);
    } else {
        app.SetAction(m_userIndex, eAppAction_ExitWorld, (void*)true);
    }

    
    
    
}

void ClientConnection::sendAndDisconnect(std::shared_ptr<Packet> packet) {
    if (done) return;
    connection->send(packet);
    connection->sendAndQuit();
}

void ClientConnection::send(std::shared_ptr<Packet> packet) {
    if (done) return;
    connection->send(packet);
}

void ClientConnection::handleTakeItemEntity(
    std::shared_ptr<TakeItemEntityPacket> packet) {
    std::shared_ptr<Entity> from = getEntity(packet->itemId);
    std::shared_ptr<LivingEntity> to =
        std::dynamic_pointer_cast<LivingEntity>(getEntity(packet->playerId));

    
    
    bool isLocalPlayer = false;
    for (int i = 0; i < XUSER_MAX_COUNT; i++) {
        if (minecraft->localplayers[i]) {
            if (minecraft->localplayers[i]->entityId == packet->playerId) {
                isLocalPlayer = true;
                to = minecraft->localplayers[i];
                break;
            }
        }
    }

    if (to == nullptr) {
        
        
        
        level->removeEntity(packet->itemId);
        return;
    }

    if (from != nullptr) {
        
        
        
        
        
        
        if (isLocalPlayer) {
            std::shared_ptr<LocalPlayer> player =
                std::dynamic_pointer_cast<LocalPlayer>(to);

            
            
            
            
            int playerPad = player->GetXboxPad();

            if (minecraft->localgameModes[playerPad] != nullptr) {
                
                if (from->GetType() == eTYPE_EXPERIENCEORB) {
                    float fPitch =
                        ((random->nextFloat() - random->nextFloat()) * 0.7f +
                         1.0f) *
                        2.0f;
                    app.DebugPrintf("XP Orb with pitch %f\n", fPitch);
                    level->playSound(from, eSoundType_RANDOM_ORB, 0.2f, fPitch);
                } else {
                    level->playSound(
                        from, eSoundType_RANDOM_POP, 0.2f,
                        ((random->nextFloat() - random->nextFloat()) * 0.7f +
                         1.0f) *
                            2.0f);
                }

                minecraft->particleEngine->add(
                    std::shared_ptr<TakeAnimationParticle>(
                        new TakeAnimationParticle(minecraft->level, from, to,
                                                  -0.5f)));
                level->removeEntity(packet->itemId);
            } else {
                
                
                
                level->removeEntity(packet->itemId);
            }
        } else {
            level->playSound(
                from, eSoundType_RANDOM_POP, 0.2f,
                ((random->nextFloat() - random->nextFloat()) * 0.7f + 1.0f) *
                    2.0f);
            minecraft->particleEngine->add(
                std::shared_ptr<TakeAnimationParticle>(
                    new TakeAnimationParticle(minecraft->level, from, to,
                                              -0.5f)));
            level->removeEntity(packet->itemId);
        }
    }
}

void ClientConnection::handleChat(std::shared_ptr<ChatPacket> packet) {
    std::wstring message;
    int iPos;
    bool displayOnGui = true;

    bool replacePlayer = false;
    bool replaceEntitySource = false;
    bool replaceItem = false;

    std::wstring playerDisplayName = L"";
    std::wstring sourceDisplayName = L"";

    
    if (packet->m_stringArgs.size() >= 1)
        playerDisplayName = GetDisplayNameByGamertag(packet->m_stringArgs[0]);
    if (packet->m_stringArgs.size() >= 2)
        sourceDisplayName = GetDisplayNameByGamertag(packet->m_stringArgs[1]);

    switch (packet->m_messageType) {
        case ChatPacket::e_ChatCustom:
            if (packet->m_stringArgs.size() >= 2) {
                message = L"<" + playerDisplayName + L"> " + packet->m_stringArgs[1];
            } else {
                message = packet->m_stringArgs[0];
            }
            break;
        case ChatPacket::e_ChatBedOccupied:
            message = app.GetString(IDS_TILE_BED_OCCUPIED);
            break;
        case ChatPacket::e_ChatBedNoSleep:
            message = app.GetString(IDS_TILE_BED_NO_SLEEP);
            break;
        case ChatPacket::e_ChatBedNotValid:
            message = app.GetString(IDS_TILE_BED_NOT_VALID);
            break;
        case ChatPacket::e_ChatBedNotSafe:
            message = app.GetString(IDS_TILE_BED_NOTSAFE);
            break;
        case ChatPacket::e_ChatBedPlayerSleep:
            message = app.GetString(IDS_TILE_BED_PLAYERSLEEP);
            iPos = message.find(L"%s");
            message.replace(iPos, 2, playerDisplayName);
            break;
        case ChatPacket::e_ChatBedMeSleep:
            message = app.GetString(IDS_TILE_BED_MESLEEP);
            break;
        case ChatPacket::e_ChatPlayerJoinedGame:
            message = app.GetString(IDS_PLAYER_JOINED);
            iPos = message.find(L"%s");
            message.replace(iPos, 2, playerDisplayName);
            break;
        case ChatPacket::e_ChatPlayerLeftGame:
            message = app.GetString(IDS_PLAYER_LEFT);
            iPos = message.find(L"%s");
            message.replace(iPos, 2, playerDisplayName);
            break;
        case ChatPacket::e_ChatPlayerKickedFromGame:
            message = app.GetString(IDS_PLAYER_KICKED);
            iPos = message.find(L"%s");
            message.replace(iPos, 2, playerDisplayName);
            break;
        case ChatPacket::e_ChatCannotPlaceLava:
            displayOnGui = false;
            app.SetGlobalXuiAction(eAppAction_DisplayLavaMessage);
            break;
        case ChatPacket::e_ChatDeathInFire:
            message = app.GetString(IDS_DEATH_INFIRE);
            replacePlayer = true;
            break;
        case ChatPacket::e_ChatDeathOnFire:
            message = app.GetString(IDS_DEATH_ONFIRE);
            replacePlayer = true;
            break;
        case ChatPacket::e_ChatDeathLava:
            message = app.GetString(IDS_DEATH_LAVA);
            replacePlayer = true;
            break;
        case ChatPacket::e_ChatDeathInWall:
            message = app.GetString(IDS_DEATH_INWALL);
            replacePlayer = true;
            break;
        case ChatPacket::e_ChatDeathDrown:
            message = app.GetString(IDS_DEATH_DROWN);
            replacePlayer = true;
            break;
        case ChatPacket::e_ChatDeathStarve:
            message = app.GetString(IDS_DEATH_STARVE);
            replacePlayer = true;
            break;
        case ChatPacket::e_ChatDeathCactus:
            message = app.GetString(IDS_DEATH_CACTUS);
            replacePlayer = true;
            break;
        case ChatPacket::e_ChatDeathFall:
            message = app.GetString(IDS_DEATH_FALL);
            replacePlayer = true;
            break;
        case ChatPacket::e_ChatDeathOutOfWorld:
            message = app.GetString(IDS_DEATH_OUTOFWORLD);
            replacePlayer = true;
            break;
        case ChatPacket::e_ChatDeathGeneric:
            message = app.GetString(IDS_DEATH_GENERIC);
            replacePlayer = true;
            break;
        case ChatPacket::e_ChatDeathExplosion:
            message = app.GetString(IDS_DEATH_EXPLOSION);
            replacePlayer = true;
            break;
        case ChatPacket::e_ChatDeathMagic:
            message = app.GetString(IDS_DEATH_MAGIC);
            replacePlayer = true;
            break;
        case ChatPacket::e_ChatDeathAnvil:
            message = app.GetString(IDS_DEATH_FALLING_ANVIL);
            replacePlayer = true;
            break;
        case ChatPacket::e_ChatDeathFallingBlock:
            message = app.GetString(IDS_DEATH_FALLING_TILE);
            replacePlayer = true;
            break;
        case ChatPacket::e_ChatDeathDragonBreath:
            message = app.GetString(IDS_DEATH_DRAGON_BREATH);
            replacePlayer = true;
            break;
        case ChatPacket::e_ChatDeathMob:
            message = app.GetString(IDS_DEATH_MOB);
            replacePlayer = true;
            replaceEntitySource = true;
            break;
        case ChatPacket::e_ChatDeathPlayer:
            message = app.GetString(IDS_DEATH_PLAYER);
            replacePlayer = true;
            replaceEntitySource = true;
            break;
        case ChatPacket::e_ChatDeathArrow:
            message = app.GetString(IDS_DEATH_ARROW);
            replacePlayer = true;
            replaceEntitySource = true;
            break;
        case ChatPacket::e_ChatDeathFireball:
            message = app.GetString(IDS_DEATH_FIREBALL);
            replacePlayer = true;
            replaceEntitySource = true;
            break;
        case ChatPacket::e_ChatDeathThrown:
            message = app.GetString(IDS_DEATH_THROWN);
            replacePlayer = true;
            replaceEntitySource = true;
            break;
        case ChatPacket::e_ChatDeathIndirectMagic:
            message = app.GetString(IDS_DEATH_INDIRECT_MAGIC);
            replacePlayer = true;
            replaceEntitySource = true;
            break;
        case ChatPacket::e_ChatDeathThorns:
            message = app.GetString(IDS_DEATH_THORNS);
            replacePlayer = true;
            replaceEntitySource = true;
            break;

        case ChatPacket::e_ChatDeathFellAccidentLadder:
            message = app.GetString(IDS_DEATH_FELL_ACCIDENT_LADDER);
            replacePlayer = true;
            break;
        case ChatPacket::e_ChatDeathFellAccidentVines:
            message = app.GetString(IDS_DEATH_FELL_ACCIDENT_VINES);
            replacePlayer = true;
            break;
        case ChatPacket::e_ChatDeathFellAccidentWater:
            message = app.GetString(IDS_DEATH_FELL_ACCIDENT_WATER);
            replacePlayer = true;
            break;
        case ChatPacket::e_ChatDeathFellAccidentGeneric:
            message = app.GetString(IDS_DEATH_FELL_ACCIDENT_GENERIC);
            replacePlayer = true;
            break;
        case ChatPacket::e_ChatDeathFellKiller:
            
            
            

            
            
            
            message = app.GetString(IDS_DEATH_FALL);
            replacePlayer = true;
            break;
        case ChatPacket::e_ChatDeathFellAssist:
            message = app.GetString(IDS_DEATH_FELL_ASSIST);
            replacePlayer = true;
            replaceEntitySource = true;
            break;
        case ChatPacket::e_ChatDeathFellAssistItem:
            message = app.GetString(IDS_DEATH_FELL_ASSIST_ITEM);
            replacePlayer = true;
            replaceEntitySource = true;
            replaceItem = true;
            break;
        case ChatPacket::e_ChatDeathFellFinish:
            message = app.GetString(IDS_DEATH_FELL_FINISH);
            replacePlayer = true;
            replaceEntitySource = true;
            break;
        case ChatPacket::e_ChatDeathFellFinishItem:
            message = app.GetString(IDS_DEATH_FELL_FINISH_ITEM);
            replacePlayer = true;
            replaceEntitySource = true;
            replaceItem = true;
            break;
        case ChatPacket::e_ChatDeathInFirePlayer:
            message = app.GetString(IDS_DEATH_INFIRE_PLAYER);
            replacePlayer = true;
            replaceEntitySource = true;
            break;
        case ChatPacket::e_ChatDeathOnFirePlayer:
            message = app.GetString(IDS_DEATH_ONFIRE_PLAYER);
            replacePlayer = true;
            replaceEntitySource = true;
            break;
        case ChatPacket::e_ChatDeathLavaPlayer:
            message = app.GetString(IDS_DEATH_LAVA_PLAYER);
            replacePlayer = true;
            replaceEntitySource = true;
            break;
        case ChatPacket::e_ChatDeathDrownPlayer:
            message = app.GetString(IDS_DEATH_DROWN_PLAYER);
            replacePlayer = true;
            replaceEntitySource = true;
            break;
        case ChatPacket::e_ChatDeathCactusPlayer:
            message = app.GetString(IDS_DEATH_CACTUS_PLAYER);
            replacePlayer = true;
            replaceEntitySource = true;
            break;
        case ChatPacket::e_ChatDeathExplosionPlayer:
            message = app.GetString(IDS_DEATH_EXPLOSION_PLAYER);
            replacePlayer = true;
            replaceEntitySource = true;
            break;
        case ChatPacket::e_ChatDeathWither:
            message = app.GetString(IDS_DEATH_WITHER);
            replacePlayer = true;
            break;
        case ChatPacket::e_ChatDeathPlayerItem:
            message = app.GetString(IDS_DEATH_PLAYER_ITEM);
            replacePlayer = true;
            replaceEntitySource = true;
            replaceItem = true;
            break;
        case ChatPacket::e_ChatDeathArrowItem:
            message = app.GetString(IDS_DEATH_ARROW_ITEM);
            replacePlayer = true;
            replaceEntitySource = true;
            replaceItem = true;
            break;
        case ChatPacket::e_ChatDeathFireballItem:
            message = app.GetString(IDS_DEATH_FIREBALL_ITEM);
            replacePlayer = true;
            replaceEntitySource = true;
            replaceItem = true;
            break;
        case ChatPacket::e_ChatDeathThrownItem:
            message = app.GetString(IDS_DEATH_THROWN_ITEM);
            replacePlayer = true;
            replaceEntitySource = true;
            replaceItem = true;
            break;
        case ChatPacket::e_ChatDeathIndirectMagicItem:
            message = app.GetString(IDS_DEATH_INDIRECT_MAGIC_ITEM);
            replacePlayer = true;
            replaceEntitySource = true;
            replaceItem = true;
            break;

        case ChatPacket::e_ChatPlayerEnteredEnd:
            message = app.GetString(IDS_PLAYER_ENTERED_END);
            iPos = message.find(L"%s");
            message.replace(iPos, 2, playerDisplayName);
            break;
        case ChatPacket::e_ChatPlayerLeftEnd:
            message = app.GetString(IDS_PLAYER_LEFT_END);
            iPos = message.find(L"%s");
            message.replace(iPos, 2, playerDisplayName);
            break;

        case ChatPacket::e_ChatPlayerMaxEnemies:
            message = app.GetString(IDS_MAX_ENEMIES_SPAWNED);
            break;
            
        case ChatPacket::e_ChatPlayerMaxVillagers:
            message = app.GetString(IDS_MAX_VILLAGERS_SPAWNED);
            break;
        case ChatPacket::e_ChatPlayerMaxPigsSheepCows:
            message = app.GetString(IDS_MAX_PIGS_SHEEP_COWS_CATS_SPAWNED);
            break;
        case ChatPacket::e_ChatPlayerMaxChickens:
            message = app.GetString(IDS_MAX_CHICKENS_SPAWNED);
            break;
        case ChatPacket::e_ChatPlayerMaxSquid:
            message = app.GetString(IDS_MAX_SQUID_SPAWNED);
            break;
        case ChatPacket::e_ChatPlayerMaxMooshrooms:
            message = app.GetString(IDS_MAX_MOOSHROOMS_SPAWNED);
            break;
        case ChatPacket::e_ChatPlayerMaxWolves:
            message = app.GetString(IDS_MAX_WOLVES_SPAWNED);
            break;
        case ChatPacket::e_ChatPlayerMaxBats:
            message = app.GetString(IDS_MAX_BATS_SPAWNED);
            break;

            
        case ChatPacket::e_ChatPlayerMaxBredPigsSheepCows:
            message = app.GetString(IDS_MAX_PIGS_SHEEP_COWS_CATS_BRED);
            break;
        case ChatPacket::e_ChatPlayerMaxBredChickens:
            message = app.GetString(IDS_MAX_CHICKENS_BRED);
            break;
        case ChatPacket::e_ChatPlayerMaxBredMooshrooms:
            message = app.GetString(IDS_MAX_MUSHROOMCOWS_BRED);
            break;

        case ChatPacket::e_ChatPlayerMaxBredWolves:
            message = app.GetString(IDS_MAX_WOLVES_BRED);
            break;

            
        case ChatPacket::e_ChatPlayerCantShearMooshroom:
            message = app.GetString(IDS_CANT_SHEAR_MOOSHROOM);
            break;

            
        case ChatPacket::e_ChatPlayerMaxHangingEntities:
            message = app.GetString(IDS_MAX_HANGINGENTITIES);
            break;
            
        case ChatPacket::e_ChatPlayerCantSpawnInPeaceful:
            message = app.GetString(IDS_CANT_SPAWN_IN_PEACEFUL);
            break;

            
        case ChatPacket::e_ChatPlayerMaxBoats:
            message = app.GetString(IDS_MAX_BOATS);
            break;

        case ChatPacket::e_ChatCommandTeleportSuccess:
            message = app.GetString(IDS_COMMAND_TELEPORT_SUCCESS);
            replacePlayer = true;
            if (packet->m_intArgs[0] == eTYPE_SERVERPLAYER) {
                message =
                    replaceAll(message, L"{*DESTINATION*}", sourceDisplayName);
            } else {
                message = replaceAll(
                    message, L"{*DESTINATION*}",
                    app.getEntityName((eINSTANCEOF)packet->m_intArgs[0]));
            }
            break;
        case ChatPacket::e_ChatCommandTeleportMe:
            message = app.GetString(IDS_COMMAND_TELEPORT_ME);
            replacePlayer = true;
            break;
        case ChatPacket::e_ChatCommandTeleportToMe:
            message = app.GetString(IDS_COMMAND_TELEPORT_TO_ME);
            replacePlayer = true;
            break;

        default:
            message = playerDisplayName;
            break;
    }

    if (replacePlayer) {
        message = replaceAll(message, L"{*PLAYER*}", playerDisplayName);
    }

    if (replaceEntitySource) {
        if (packet->m_intArgs[0] == eTYPE_SERVERPLAYER) {
            message = replaceAll(message, L"{*SOURCE*}", sourceDisplayName);
        } else {
            std::wstring entityName;

            
            if (packet->m_stringArgs.size() >= 2 &&
                !packet->m_stringArgs[1].empty()) {
                entityName = packet->m_stringArgs[1];
            } else {
                entityName =
                    app.getEntityName((eINSTANCEOF)packet->m_intArgs[0]);
            }

            message = replaceAll(message, L"{*SOURCE*}", entityName);
        }
    }

    if (replaceItem) {
        message = replaceAll(message, L"{*ITEM*}", packet->m_stringArgs[2]);
    }

    
    bool bIsDeathMessage =
        (packet->m_messageType >= ChatPacket::e_ChatDeathInFire) &&
        (packet->m_messageType <= ChatPacket::e_ChatDeathIndirectMagicItem);

    if (displayOnGui)
        minecraft->gui->addMessage(message, m_userIndex, bIsDeathMessage);
}

void ClientConnection::handleAnimate(std::shared_ptr<AnimatePacket> packet) {
    std::shared_ptr<Entity> e = getEntity(packet->id);
    if (e == nullptr) return;
    if (packet->action == AnimatePacket::SWING) {
        if (e->instanceof(eTYPE_LIVINGENTITY))
            std::dynamic_pointer_cast<LivingEntity>(e)->swing();
    } else if (packet->action == AnimatePacket::HURT) {
        e->animateHurt();
    } else if (packet->action == AnimatePacket::WAKE_UP) {
        if (e->instanceof(eTYPE_PLAYER))
            std::dynamic_pointer_cast<Player>(e)->stopSleepInBed(false, false,
                                                                 false);
    } else if (packet->action == AnimatePacket::RESPAWN) {
    } else if (packet->action == AnimatePacket::CRITICAL_HIT) {
        std::shared_ptr<CritParticle> critParticle =
            std::shared_ptr<CritParticle>(
                new CritParticle(minecraft->level, e));
        critParticle->CritParticlePostConstructor();
        minecraft->particleEngine->add(critParticle);
    } else if (packet->action == AnimatePacket::MAGIC_CRITICAL_HIT) {
        std::shared_ptr<CritParticle> critParticle =
            std::shared_ptr<CritParticle>(
                new CritParticle(minecraft->level, e, eParticleType_magicCrit));
        critParticle->CritParticlePostConstructor();
        minecraft->particleEngine->add(critParticle);
    } else if ((packet->action == AnimatePacket::EAT) &&
               e->instanceof(eTYPE_REMOTEPLAYER)) {
    }
}

void ClientConnection::handleEntityActionAtPosition(
    std::shared_ptr<EntityActionAtPositionPacket> packet) {
    std::shared_ptr<Entity> e = getEntity(packet->id);
    if (e == nullptr) return;
    if (packet->action == EntityActionAtPositionPacket::START_SLEEP) {
        std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(e);
        player->startSleepInBed(packet->x, packet->y, packet->z);
    }
}

void ClientConnection::handlePreLogin(std::shared_ptr<PreLoginPacket> packet) {
    fprintf(stderr,
            "[LOGIN-CLI] handlePreLogin entered, isHost=%d, userIdx=%d\n",
            (int)g_NetworkManager.IsHost(), m_userIndex);
    
    
    bool canPlay = true;
    bool canPlayLocal = true;
    bool isAtLeastOneFriend = g_NetworkManager.IsHost();
    bool isFriendsWithHost = true;
    bool cantPlayContentRestricted = false;

    if (!g_NetworkManager.IsHost()) {
        
        app.SetGameHostOption(eGameHostOption_All, packet->m_serverSettings);

        
        
        if (app.GetTMSGlobalFileListRead() == false) {
            app.SetTMSAction(InputManager.GetPrimaryPad(),
                             eTMSAction_TMSPP_RetrieveFiles_RunPlayGame);
        }
    }

    
    canPlay = true;
    canPlayLocal = true;
    isAtLeastOneFriend = true;
    cantPlayContentRestricted = false;

    if (!canPlay || !canPlayLocal || !isAtLeastOneFriend ||
        cantPlayContentRestricted) {
        DisconnectPacket::eDisconnectReason reason =
            DisconnectPacket::eDisconnect_NoUGC_Remote;
        if (m_userIndex == InputManager.GetPrimaryPad()) {
            if (!isFriendsWithHost)
                reason = DisconnectPacket::eDisconnect_NotFriendsWithHost;
            else if (!isAtLeastOneFriend)
                reason = DisconnectPacket::eDisconnect_NoFriendsInGame;
            else if (!canPlayLocal)
                reason = DisconnectPacket::eDisconnect_NoUGC_AllLocal;
            else if (cantPlayContentRestricted)
                reason =
                    DisconnectPacket::eDisconnect_ContentRestricted_AllLocal;

            app.DebugPrintf(
                "Exiting world on handling Pre-Login packet due UGC "
                "privileges: %d\n",
                reason);
            app.SetDisconnectReason(reason);
            app.SetAction(InputManager.GetPrimaryPad(), eAppAction_ExitWorld,
                          (void*)true);
        } else {
            if (!isFriendsWithHost)
                reason = DisconnectPacket::eDisconnect_NotFriendsWithHost;
            else if (!canPlayLocal)
                reason = DisconnectPacket::eDisconnect_NoUGC_Single_Local;
            else if (cantPlayContentRestricted)
                reason = DisconnectPacket::
                    eDisconnect_ContentRestricted_Single_Local;

            app.DebugPrintf(
                "Exiting player %d on handling Pre-Login packet due UGC "
                "privileges: %d\n",
                m_userIndex, reason);
            unsigned int uiIDA[1];
            uiIDA[0] = IDS_CONFIRM_OK;
            if (!isFriendsWithHost)
                ui.RequestErrorMessage(IDS_CANTJOIN_TITLE,
                                       IDS_NOTALLOWED_FRIENDSOFFRIENDS, uiIDA,
                                       1, m_userIndex);
            else
                ui.RequestErrorMessage(
                    IDS_CANTJOIN_TITLE,
                    IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_SINGLE_LOCAL, uiIDA,
                    1, m_userIndex);

            app.SetDisconnectReason(reason);

            
            
            
            
            
            
            

            
            app.SetAction(m_userIndex, eAppAction_ExitPlayerPreLogin);
        }
    } else {
        
        
        
        
        
        
        if (m_userIndex == InputManager.GetPrimaryPad()) {
            Minecraft* pMinecraft = Minecraft::GetInstance();
            if (pMinecraft->skins->selectTexturePackById(
                    packet->m_texturePackId)) {
                app.DebugPrintf(
                    "Selected texture pack %d from Pre-Login packet\n",
                    packet->m_texturePackId);
            } else {
                app.DebugPrintf(
                    "Could not select texture pack %d from Pre-Login packet, "
                    "requesting from host\n",
                    packet->m_texturePackId);

                
                
                
            }
        }

        if (!g_NetworkManager.IsHost()) {
            Minecraft::GetInstance()->progressRenderer->progressStagePercentage(
                (eCCPreLoginReceived * 100) / (eCCConnected));
        }
        
        PlayerUID offlineXUID = INVALID_XUID;
        PlayerUID onlineXUID = INVALID_XUID;
        if (ProfileManager.IsSignedInLive(m_userIndex)) {
            
            
            ProfileManager.GetXUID(m_userIndex, &onlineXUID, true);
        }

        
        
        if (!ProfileManager.IsGuest(m_userIndex)) {
            
            
            ProfileManager.GetXUID(m_userIndex, &offlineXUID, false);
        }
        bool allAllowed = false;
        bool friendsAllowed = false;
        ProfileManager.AllowedPlayerCreatedContent(
            m_userIndex, true, &allAllowed, &friendsAllowed);
        fprintf(stderr,
                "[LOGIN] Sending LoginPacket: user=%ls netVer=%d userIdx=%d "
                "isHost=%d\n",
                minecraft->user->name.c_str(),
                SharedConstants::NETWORK_PROTOCOL_VERSION, m_userIndex,
                (int)g_NetworkManager.IsHost());
        send(std::make_shared<LoginPacket>(
            minecraft->user->name, SharedConstants::NETWORK_PROTOCOL_VERSION,
            offlineXUID, onlineXUID, (!allAllowed && friendsAllowed),
            packet->m_ugcPlayersVersion, app.GetPlayerSkinId(m_userIndex),
            app.GetPlayerCapeId(m_userIndex),
            ProfileManager.IsGuest(m_userIndex)));
        fprintf(stderr, "[LOGIN] LoginPacket sent successfully\n");

        if (!g_NetworkManager.IsHost()) {
            Minecraft::GetInstance()->progressRenderer->progressStagePercentage(
                (eCCLoginSent * 100) / (eCCConnected));
        }
    }
}

void ClientConnection::close() {
    
    
    if (done) return;
    done = true;
    connection->flush();
    connection->close(DisconnectPacket::eDisconnect_Closed);
}

void ClientConnection::handleAddMob(std::shared_ptr<AddMobPacket> packet) {
    double x = packet->x / 32.0;
    double y = packet->y / 32.0;
    double z = packet->z / 32.0;
    float yRot = packet->yRot * 360 / 256.0f;
    float xRot = packet->xRot * 360 / 256.0f;

    std::shared_ptr<LivingEntity> mob = std::dynamic_pointer_cast<LivingEntity>(
        EntityIO::newById(packet->type, level));
    if (mob == nullptr) {
        return;
    }
    mob->xp = packet->x;
    mob->yp = packet->y;
    mob->zp = packet->z;
    mob->yHeadRot = packet->yHeadRot * 360 / 256.0f;
    mob->yRotp = packet->yRot;
    mob->xRotp = packet->xRot;

    std::vector<std::shared_ptr<Entity> >* subEntities = mob->getSubEntities();
    if (subEntities != nullptr) {
        int offs = packet->id - mob->entityId;
        
        for (auto it = subEntities->begin(); it != subEntities->end(); ++it) {
            
            (*it)->entityId += offs;
        }
    }

    mob->entityId = packet->id;

    

    mob->absMoveTo(x, y, z, yRot, xRot);
    mob->xd = packet->xd / 8000.0f;
    mob->yd = packet->yd / 8000.0f;
    mob->zd = packet->zd / 8000.0f;
    level->putEntity(packet->id, mob);

    std::vector<std::shared_ptr<SynchedEntityData::DataItem> >* unpackedData =
        packet->getUnpackedData();
    if (unpackedData != nullptr) {
        mob->getEntityData()->assignValues(unpackedData);
    }

    
    
    
    if (mob->GetType() == eTYPE_SLIME || mob->GetType() == eTYPE_LAVASLIME) {
        std::shared_ptr<Slime> slime = std::dynamic_pointer_cast<Slime>(mob);
        slime->setSize(slime->getSize());
    }
}

void ClientConnection::handleSetTime(std::shared_ptr<SetTimePacket> packet) {
    minecraft->level->setGameTime(packet->gameTime);
    minecraft->level->setDayTime(packet->dayTime);
}

void ClientConnection::handleSetSpawn(
    std::shared_ptr<SetSpawnPositionPacket> packet) {
    
    
    minecraft->localplayers[m_userIndex]->setRespawnPosition(
        new Pos(packet->x, packet->y, packet->z), true);
    minecraft->level->getLevelData()->setSpawn(packet->x, packet->y, packet->z);
}

void ClientConnection::handleEntityLinkPacket(
    std::shared_ptr<SetEntityLinkPacket> packet) {
    std::shared_ptr<Entity> sourceEntity = getEntity(packet->sourceId);
    std::shared_ptr<Entity> destEntity = getEntity(packet->destId);

    
    
    
    if (destEntity == nullptr && packet->destId >= 0) {
        
        assert(!(sourceEntity == nullptr && packet->sourceId >= 0));

        deferredEntityLinkPackets.push_back(DeferredEntityLinkPacket(packet));
        return;
    }

    if (packet->type == SetEntityLinkPacket::RIDING) {
        bool displayMountMessage = false;
        if (packet->sourceId == Minecraft::GetInstance()
                                    ->localplayers[m_userIndex]
                                    .get()
                                    ->entityId) {
            sourceEntity = Minecraft::GetInstance()->localplayers[m_userIndex];

            if (destEntity != nullptr && destEntity->instanceof(eTYPE_BOAT))
                (std::dynamic_pointer_cast<Boat>(destEntity))->setDoLerp(false);

            displayMountMessage =
                (sourceEntity->riding == nullptr && destEntity != nullptr);
        } else if (destEntity != nullptr &&
                   destEntity->instanceof(eTYPE_BOAT)) {
            (std::dynamic_pointer_cast<Boat>(destEntity))->setDoLerp(true);
        }

        if (sourceEntity == nullptr) return;

        sourceEntity->ride(destEntity);

        
        






    } else if (packet->type == SetEntityLinkPacket::LEASH) {
        if ((sourceEntity != nullptr) && sourceEntity->instanceof(eTYPE_MOB)) {
            if (destEntity != nullptr) {
                (std::dynamic_pointer_cast<Mob>(sourceEntity))
                    ->setLeashedTo(destEntity, false);
            } else {
                (std::dynamic_pointer_cast<Mob>(sourceEntity))
                    ->dropLeash(false, false);
            }
        }
    }
}

void ClientConnection::handleEntityEvent(
    std::shared_ptr<EntityEventPacket> packet) {
    std::shared_ptr<Entity> e = getEntity(packet->entityId);
    if (e != nullptr) e->handleEntityEvent(packet->eventId);
}

std::shared_ptr<Entity> ClientConnection::getEntity(int entityId) {
    
    if (entityId == minecraft->localplayers[m_userIndex]->entityId) {
        
        return minecraft->localplayers[m_userIndex];
    }
    return level->getEntity(entityId);
}

void ClientConnection::handleSetHealth(
    std::shared_ptr<SetHealthPacket> packet) {
    
    minecraft->localplayers[m_userIndex]->hurtTo(packet->health,
                                                 packet->damageSource);
    minecraft->localplayers[m_userIndex]->getFoodData()->setFoodLevel(
        packet->food);
    minecraft->localplayers[m_userIndex]->getFoodData()->setSaturation(
        packet->saturation);

    
    if (packet->food < FoodConstants::HEAL_LEVEL - 1) {
        if (minecraft->localgameModes[m_userIndex] != nullptr &&
            !minecraft->localgameModes[m_userIndex]->hasInfiniteItems()) {
            minecraft->localgameModes[m_userIndex]
                ->getTutorial()
                ->changeTutorialState(e_Tutorial_State_Food_Bar);
        }
    }
}

void ClientConnection::handleSetExperience(
    std::shared_ptr<SetExperiencePacket> packet) {
    minecraft->localplayers[m_userIndex]->setExperienceValues(
        packet->experienceProgress, packet->totalExperience,
        packet->experienceLevel);
}

void ClientConnection::handleTexture(std::shared_ptr<TexturePacket> packet) {
    
    
    
    

    if (packet->dataBytes == 0) {
        
#if !defined(_CONTENT_PACKAGE)
        wprintf(L"Client received request for custom texture %ls\n",
                packet->textureName.c_str());
#endif
        std::uint8_t* pbData = nullptr;
        unsigned int dwBytes = 0;
        app.GetMemFileDetails(packet->textureName, &pbData, &dwBytes);

        if (dwBytes != 0) {
            send(std::shared_ptr<TexturePacket>(
                new TexturePacket(packet->textureName, pbData, dwBytes)));
        }
    } else {
        
#if !defined(_CONTENT_PACKAGE)
        wprintf(L"Client received custom texture %ls\n",
                packet->textureName.c_str());
#endif
        app.AddMemoryTextureFile(packet->textureName, packet->pbData,
                                 packet->dataBytes);
        Minecraft::GetInstance()->handleClientTextureReceived(
            packet->textureName);
    }
}

void ClientConnection::handleTextureAndGeometry(
    std::shared_ptr<TextureAndGeometryPacket> packet) {
    
    
    
    

    if (packet->dwTextureBytes == 0) {
        
#if !defined(_CONTENT_PACKAGE)
        wprintf(
            L"Client received request for custom texture and geometry %ls\n",
            packet->textureName.c_str());
#endif
        std::uint8_t* pbData = nullptr;
        unsigned int dwBytes = 0;
        app.GetMemFileDetails(packet->textureName, &pbData, &dwBytes);
        DLCSkinFile* pDLCSkinFile =
            app.m_dlcManager.getSkinFile(packet->textureName);

        if (dwBytes != 0) {
            if (pDLCSkinFile) {
                if (pDLCSkinFile->getAdditionalBoxesCount() != 0) {
                    send(std::shared_ptr<TextureAndGeometryPacket>(
                        new TextureAndGeometryPacket(packet->textureName,
                                                     pbData, dwBytes,
                                                     pDLCSkinFile)));
                } else {
                    send(std::shared_ptr<TextureAndGeometryPacket>(
                        new TextureAndGeometryPacket(packet->textureName,
                                                     pbData, dwBytes)));
                }
            } else {
                unsigned int uiAnimOverrideBitmask =
                    app.GetAnimOverrideBitmask(packet->dwSkinID);

                send(std::shared_ptr<TextureAndGeometryPacket>(
                    new TextureAndGeometryPacket(
                        packet->textureName, pbData, dwBytes,
                        app.GetAdditionalSkinBoxes(packet->dwSkinID),
                        uiAnimOverrideBitmask)));
            }
        }
    } else {
        
#if !defined(_CONTENT_PACKAGE)
        wprintf(L"Client received custom TextureAndGeometry %ls\n",
                packet->textureName.c_str());
#endif
        
        app.AddMemoryTextureFile(packet->textureName, packet->pbData,
                                 packet->dwTextureBytes);
        
        if (packet->dwBoxC != 0) {
            app.SetAdditionalSkinBoxes(packet->dwSkinID, packet->BoxDataA,
                                       packet->dwBoxC);
        }
        
        app.SetAnimOverrideBitmask(packet->dwSkinID,
                                   packet->uiAnimOverrideBitmask);

        
        Minecraft::GetInstance()->handleClientTextureReceived(
            packet->textureName);
    }
}

void ClientConnection::handleTextureChange(
    std::shared_ptr<TextureChangePacket> packet) {
    std::shared_ptr<Entity> e = getEntity(packet->id);
    fprintf(stderr,
            "[JNPC] TextureChange target id=%d class=%s player=%d mob=%d "
            "pig=%d path=%ls\n",
            packet->id, e ? typeid(*e).name() : "(none)",
            e ? (int)e->instanceof(eTYPE_PLAYER) : -1,
            e ? (int)e->instanceof(eTYPE_MOB) : -1,
            e ? (int)e->instanceof(eTYPE_PIG) : -1, packet->path.c_str());
    if ((e == nullptr) || !e->instanceof(eTYPE_PLAYER)) return;
    std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(e);

    bool isLocalPlayer = false;
    for (int i = 0; i < XUSER_MAX_COUNT; i++) {
        if (minecraft->localplayers[i]) {
            if (minecraft->localplayers[i]->entityId == packet->id) {
                isLocalPlayer = true;
                break;
            }
        }
    }
    if (isLocalPlayer) return;

    switch (packet->action) {
        case TextureChangePacket::e_TextureChange_Skin:
            
            
            
            
            
            
            
            if (packet->path.compare(0, 7, L"dlcskin") == 0 ||
                packet->path.compare(0, 7, L"ugcskin") == 0 ||
                packet->path.compare(0, 7, L"defskin") == 0) {
                player->setCustomSkin(app.getSkinIdFromPath(packet->path));
            } else {
                player->customTextureUrl = packet->path;
            }
#if !defined(_CONTENT_PACKAGE)
            wprintf(L"Skin for remote player %ls has changed to %ls (%d)\n",
                    player->name.c_str(), player->customTextureUrl.c_str(),
                    player->getPlayerDefaultSkin());
#endif
            break;
        case TextureChangePacket::e_TextureChange_Cape:
            player->setCustomCape(Player::getCapeIdFromPath(packet->path));
            
#if !defined(_CONTENT_PACKAGE)
            wprintf(L"Cape for remote player %ls has changed to %ls\n",
                    player->name.c_str(), player->customTextureUrl2.c_str());
#endif
            break;
    }

    if (!packet->path.empty() &&
        packet->path.substr(0, 3).compare(L"def") != 0 &&
        !app.IsFileInMemoryTextures(packet->path)) {
        if (minecraft->addPendingClientTextureRequest(packet->path)) {
#if !defined(_CONTENT_PACKAGE)
            wprintf(
                L"handleTextureChange - Client sending texture packet to get "
                L"custom skin %ls for player %ls\n",
                packet->path.c_str(), player->name.c_str());
#endif
            send(std::shared_ptr<TexturePacket>(
                new TexturePacket(packet->path, nullptr, 0)));
        }
    } else if (!packet->path.empty() &&
               app.IsFileInMemoryTextures(packet->path)) {
        
        app.AddMemoryTextureFile(packet->path, nullptr, 0);
    }
}

void ClientConnection::handleTextureAndGeometryChange(
    std::shared_ptr<TextureAndGeometryChangePacket> packet) {
    std::shared_ptr<Entity> e = getEntity(packet->id);
    if (e == nullptr) return;
    std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(e);
    if (e == nullptr) return;

    bool isLocalPlayer = false;
    for (int i = 0; i < XUSER_MAX_COUNT; i++) {
        if (minecraft->localplayers[i]) {
            if (minecraft->localplayers[i]->entityId == packet->id) {
                isLocalPlayer = true;
                break;
            }
        }
    }
    if (isLocalPlayer) return;

    player->setCustomSkin(app.getSkinIdFromPath(packet->path));

#if !defined(_CONTENT_PACKAGE)
    wprintf(L"Skin for remote player %ls has changed to %ls (%d)\n",
            player->name.c_str(), player->customTextureUrl.c_str(),
            player->getPlayerDefaultSkin());
#endif

    if (!packet->path.empty() &&
        packet->path.substr(0, 3).compare(L"def") != 0 &&
        !app.IsFileInMemoryTextures(packet->path)) {
        if (minecraft->addPendingClientTextureRequest(packet->path)) {
#if !defined(_CONTENT_PACKAGE)
            wprintf(
                L"handleTextureAndGeometryChange - Client sending "
                L"TextureAndGeometryPacket to get custom skin %ls for player "
                L"%ls\n",
                packet->path.c_str(), player->name.c_str());
#endif
            send(std::shared_ptr<TextureAndGeometryPacket>(
                new TextureAndGeometryPacket(packet->path, nullptr, 0)));
        }
    } else if (!packet->path.empty() &&
               app.IsFileInMemoryTextures(packet->path)) {
        
        app.AddMemoryTextureFile(packet->path, nullptr, 0);
    }
}

void ClientConnection::handleRespawn(std::shared_ptr<RespawnPacket> packet) {
    
    if (packet->dimension != minecraft->localplayers[m_userIndex]->dimension ||
        packet->mapSeed !=
            minecraft->localplayers[m_userIndex]->level->getSeed()) {
        int oldDimension = minecraft->localplayers[m_userIndex]->dimension;
        started = false;

        
        level->removeClientConnection(this, false);

        MultiPlayerLevel* dimensionLevel =
            (MultiPlayerLevel*)minecraft->getLevel(packet->dimension);
        if (dimensionLevel == nullptr) {
            dimensionLevel = new MultiPlayerLevel(
                this,
                new LevelSettings(
                    packet->mapSeed, packet->playerGameType, false,
                    minecraft->level->getLevelData()->isHardcore(),
                    packet->m_newSeaLevel, packet->m_pLevelType,
                    packet->m_xzSize, packet->m_hellScale),
                packet->dimension, packet->difficulty);

            
            
            
            
            
            
            
            
            dimensionLevel->savedDataStorage = level->savedDataStorage;

            dimensionLevel->difficulty = packet->difficulty;  
            app.DebugPrintf("dimensionLevel->difficulty - Difficulty = %d\n",
                            packet->difficulty);

            dimensionLevel->isClientSide = true;
        } else {
            dimensionLevel->addClientConnection(this);
        }

        
        level->removeEntity(
            std::shared_ptr<Entity>(minecraft->localplayers[m_userIndex]));

        level = dimensionLevel;

        
        
        std::shared_ptr<MultiplayerLocalPlayer> lastPlayer = minecraft->player;
        minecraft->player = minecraft->localplayers[m_userIndex];
        minecraft->setLevel(dimensionLevel);
        minecraft->player = lastPlayer;

        
        minecraft->localplayers[m_userIndex]->dimension = packet->dimension;
        minecraft->setScreen(new ReceivingLevelScreen(this));
        

        if (minecraft->localgameModes[m_userIndex] != nullptr) {
            TutorialMode* gameMode =
                (TutorialMode*)minecraft->localgameModes[m_userIndex];
            gameMode->getTutorial()->showTutorialPopup(false);
        }

        
        
        minecraft->localplayers[m_userIndex]->updateRichPresence();

        ConnectionProgressParams* param = new ConnectionProgressParams();
        param->iPad = m_userIndex;
        if (packet->dimension == -1) {
            param->stringId = IDS_PROGRESS_ENTERING_NETHER;
        } else if (oldDimension == -1) {
            param->stringId = IDS_PROGRESS_LEAVING_NETHER;
        } else if (packet->dimension == 1) {
            param->stringId = IDS_PROGRESS_ENTERING_END;
        } else if (oldDimension == 1) {
            param->stringId = IDS_PROGRESS_LEAVING_END;
        }
        param->showTooltips = false;
        param->setFailTimer = false;

        
        
        ui.CloseUIScenes(m_userIndex);

        if (app.GetLocalPlayerCount() > 1) {
            ui.NavigateToScene(m_userIndex, eUIScene_ConnectingProgress, param);
        } else {
            ui.NavigateToScene(m_userIndex, eUIScene_ConnectingProgress, param);
        }

        app.SetAction(m_userIndex, eAppAction_WaitForDimensionChangeComplete);
    }

    
    

    
    
    
    int oldIndex = minecraft->getLocalPlayerIdx();
    minecraft->setLocalPlayerIdx(m_userIndex);
    minecraft->respawnPlayer(minecraft->localplayers[m_userIndex]->GetXboxPad(),
                             packet->dimension, packet->m_newEntityId);
    ((MultiPlayerGameMode*)minecraft->localgameModes[m_userIndex])
        ->setLocalMode(packet->playerGameType);
    minecraft->setLocalPlayerIdx(oldIndex);
}

void ClientConnection::handleExplosion(std::shared_ptr<ExplodePacket> packet) {
    if (!packet->m_bKnockbackOnly) {
        
        Explosion* e = new Explosion(minecraft->level, nullptr, packet->x,
                                     packet->y, packet->z, packet->r);

        
        
        
        
        
        
        MultiPlayerLevel* mpLevel = (MultiPlayerLevel*)minecraft->level;
        mpLevel->enableResetChanges(false);
        
        
        e->finalizeExplosion(true, &packet->toBlow);
        mpLevel->enableResetChanges(true);

        delete e;
    } else {
        
    }

    
    
    
    minecraft->localplayers[m_userIndex]->xd += packet->getKnockbackX();
    minecraft->localplayers[m_userIndex]->yd += packet->getKnockbackY();
    minecraft->localplayers[m_userIndex]->zd += packet->getKnockbackZ();
}

void ClientConnection::handleContainerOpen(
    std::shared_ptr<ContainerOpenPacket> packet) {
    bool failed = false;
    std::shared_ptr<MultiplayerLocalPlayer> player =
        minecraft->localplayers[m_userIndex];
    switch (packet->type) {
        case ContainerOpenPacket::BONUS_CHEST:
        case ContainerOpenPacket::LARGE_CHEST:
        case ContainerOpenPacket::ENDER_CHEST:
        case ContainerOpenPacket::CONTAINER:
        case ContainerOpenPacket::MINECART_CHEST: {
            int chestString;
            switch (packet->type) {
                case ContainerOpenPacket::MINECART_CHEST:
                    chestString = IDS_ITEM_MINECART;
                    break;
                case ContainerOpenPacket::BONUS_CHEST:
                    chestString = IDS_BONUS_CHEST;
                    break;
                case ContainerOpenPacket::LARGE_CHEST:
                    chestString = IDS_CHEST_LARGE;
                    break;
                case ContainerOpenPacket::ENDER_CHEST:
                    chestString = IDS_TILE_ENDERCHEST;
                    break;
                case ContainerOpenPacket::CONTAINER:
                    chestString = IDS_CHEST;
                    break;
                default:
                    assert(false);
                    chestString = -1;
                    break;
            }

            if (player->openContainer(std::shared_ptr<SimpleContainer>(
                    new SimpleContainer(chestString, packet->title,
                                        packet->customName, packet->size)))) {
                player->containerMenu->containerId = packet->containerId;
            } else {
                failed = true;
            }
        } break;
        case ContainerOpenPacket::HOPPER: {
            std::shared_ptr<HopperTileEntity> hopper =
                std::make_shared<HopperTileEntity>();
            if (packet->customName) hopper->setCustomName(packet->title);
            if (player->openHopper(hopper)) {
                player->containerMenu->containerId = packet->containerId;
            } else {
                failed = true;
            }
        } break;
        case ContainerOpenPacket::FURNACE: {
            std::shared_ptr<FurnaceTileEntity> furnace =
                std::make_shared<FurnaceTileEntity>();
            if (packet->customName) furnace->setCustomName(packet->title);
            if (player->openFurnace(furnace)) {
                player->containerMenu->containerId = packet->containerId;
            } else {
                failed = true;
            }
        } break;
        case ContainerOpenPacket::BREWING_STAND: {
            std::shared_ptr<BrewingStandTileEntity> brewingStand =
                std::shared_ptr<BrewingStandTileEntity>(
                    new BrewingStandTileEntity());
            if (packet->customName) brewingStand->setCustomName(packet->title);

            if (player->openBrewingStand(brewingStand)) {
                player->containerMenu->containerId = packet->containerId;
            } else {
                failed = true;
            }
        } break;
        case ContainerOpenPacket::DROPPER: {
            std::shared_ptr<DropperTileEntity> dropper =
                std::make_shared<DropperTileEntity>();
            if (packet->customName) dropper->setCustomName(packet->title);

            if (player->openTrap(dropper)) {
                player->containerMenu->containerId = packet->containerId;
            } else {
                failed = true;
            }
        } break;
        case ContainerOpenPacket::TRAP: {
            std::shared_ptr<DispenserTileEntity> dispenser =
                std::make_shared<DispenserTileEntity>();
            if (packet->customName) dispenser->setCustomName(packet->title);

            if (player->openTrap(dispenser)) {
                player->containerMenu->containerId = packet->containerId;
            } else {
                failed = true;
            }
        } break;
        case ContainerOpenPacket::WORKBENCH: {
            if (player->startCrafting(std::floor(player->x),
                                      std::floor(player->y),
                                      std::floor(player->z))) {
                player->containerMenu->containerId = packet->containerId;
            } else {
                failed = true;
            }
        } break;
        case ContainerOpenPacket::ENCHANTMENT: {
            if (player->startEnchanting(
                    std::floor(player->x), std::floor(player->y),
                    std::floor(player->z),
                    packet->customName ? packet->title : L"")) {
                player->containerMenu->containerId = packet->containerId;
            } else {
                failed = true;
            }
        } break;
        case ContainerOpenPacket::TRADER_NPC: {
            std::shared_ptr<ClientSideMerchant> csm =
                std::shared_ptr<ClientSideMerchant>(
                    new ClientSideMerchant(player, packet->title));
            csm->createContainer();
            if (player->openTrading(csm,
                                    packet->customName ? packet->title : L"")) {
                player->containerMenu->containerId = packet->containerId;
            } else {
                failed = true;
            }
        } break;
        case ContainerOpenPacket::BEACON: {
            std::shared_ptr<BeaconTileEntity> beacon =
                std::make_shared<BeaconTileEntity>();
            if (packet->customName) beacon->setCustomName(packet->title);

            if (player->openBeacon(beacon)) {
                player->containerMenu->containerId = packet->containerId;
            } else {
                failed = true;
            }
        } break;
        case ContainerOpenPacket::REPAIR_TABLE: {
            if (player->startRepairing(std::floor(player->x),
                                       std::floor(player->y),
                                       std::floor(player->z))) {
                player->containerMenu->containerId = packet->containerId;
            } else {
                failed = true;
            }
        } break;
        case ContainerOpenPacket::HORSE: {
            std::shared_ptr<EntityHorse> entity =
                std::dynamic_pointer_cast<EntityHorse>(
                    getEntity(packet->entityId));
            int iTitle = IDS_CONTAINER_ANIMAL;
            switch (entity->getType()) {
                case EntityHorse::TYPE_DONKEY:
                    iTitle = IDS_DONKEY;
                    break;
                case EntityHorse::TYPE_MULE:
                    iTitle = IDS_MULE;
                    break;
                default:
                    break;
            };
            if (player->openHorseInventory(
                    std::dynamic_pointer_cast<EntityHorse>(entity),
                    std::shared_ptr<AnimalChest>(
                        new AnimalChest(iTitle, packet->title,
                                        packet->customName, packet->size)))) {
                player->containerMenu->containerId = packet->containerId;
            } else {
                failed = true;
            }
        } break;
        case ContainerOpenPacket::FIREWORKS: {
            if (player->openFireworks(std::floor(player->x),
                                      std::floor(player->y),
                                      std::floor(player->z))) {
                player->containerMenu->containerId = packet->containerId;
            } else {
                failed = true;
            }
        } break;
    }

    if (failed) {
        
        
        
        
        
        
        if (player->containerMenu != player->inventoryMenu) {
            ui.CloseUIScenes(m_userIndex);
        } else {
            send(std::shared_ptr<ContainerClosePacket>(
                new ContainerClosePacket(packet->containerId)));
        }
    }
}

void ClientConnection::handleContainerSetSlot(
    std::shared_ptr<ContainerSetSlotPacket> packet) {
    std::shared_ptr<MultiplayerLocalPlayer> player =
        minecraft->localplayers[m_userIndex];
    if (packet->containerId == AbstractContainerMenu::CONTAINER_ID_CARRIED) {
        player->inventory->setCarried(packet->item);
    } else {
        if (packet->containerId ==
            AbstractContainerMenu::CONTAINER_ID_INVENTORY) {
            
            
            
            if (packet->slot >= 36 && packet->slot < 36 + 9) {
                std::shared_ptr<ItemInstance> lastItem =
                    player->inventoryMenu->getSlot(packet->slot)->getItem();
                if (packet->item != nullptr) {
                    if (lastItem == nullptr ||
                        lastItem->count < packet->item->count) {
                        packet->item->popTime = Inventory::POP_TIME_DURATION;
                    }
                }
            }
            player->inventoryMenu->setItem(packet->slot, packet->item);
        } else if (packet->containerId == player->containerMenu->containerId) {
            player->containerMenu->setItem(packet->slot, packet->item);
        }
    }
}

void ClientConnection::handleContainerAck(
    std::shared_ptr<ContainerAckPacket> packet) {
    std::shared_ptr<MultiplayerLocalPlayer> player =
        minecraft->localplayers[m_userIndex];
    AbstractContainerMenu* menu = nullptr;
    if (packet->containerId == AbstractContainerMenu::CONTAINER_ID_INVENTORY) {
        menu = player->inventoryMenu;
    } else if (packet->containerId == player->containerMenu->containerId) {
        menu = player->containerMenu;
    }
    if (menu != nullptr) {
        if (!packet->accepted) {
            send(std::make_shared<ContainerAckPacket>(packet->containerId,
                                                      packet->uid, true));
        }
    }
}

void ClientConnection::handleContainerContent(
    std::shared_ptr<ContainerSetContentPacket> packet) {
    std::shared_ptr<MultiplayerLocalPlayer> player =
        minecraft->localplayers[m_userIndex];
    if (packet->containerId == AbstractContainerMenu::CONTAINER_ID_INVENTORY) {
        player->inventoryMenu->setAll(&packet->items);
    } else if (packet->containerId == player->containerMenu->containerId) {
        player->containerMenu->setAll(&packet->items);
    }
}

void ClientConnection::handleTileEditorOpen(
    std::shared_ptr<TileEditorOpenPacket> packet) {
    std::shared_ptr<TileEntity> tileEntity =
        level->getTileEntity(packet->x, packet->y, packet->z);
    if (tileEntity != nullptr) {
        minecraft->localplayers[m_userIndex]->openTextEdit(tileEntity);
    } else if (packet->editorType == TileEditorOpenPacket::SIGN) {
        std::shared_ptr<SignTileEntity> localSignDummy =
            std::make_shared<SignTileEntity>();
        localSignDummy->setLevel(level);
        localSignDummy->x = packet->x;
        localSignDummy->y = packet->y;
        localSignDummy->z = packet->z;
        minecraft->player->openTextEdit(localSignDummy);
    }
}

void ClientConnection::handleSignUpdate(
    std::shared_ptr<SignUpdatePacket> packet) {
    app.DebugPrintf("ClientConnection::handleSignUpdate - ");
    if (minecraft->level->hasChunkAt(packet->x, packet->y, packet->z)) {
        std::shared_ptr<TileEntity> te =
            minecraft->level->getTileEntity(packet->x, packet->y, packet->z);

        
        if (std::dynamic_pointer_cast<SignTileEntity>(te) != nullptr) {
            std::shared_ptr<SignTileEntity> ste =
                std::dynamic_pointer_cast<SignTileEntity>(te);
            for (int i = 0; i < MAX_SIGN_LINES; i++) {
                ste->SetMessage(i, packet->lines[i]);
            }

            app.DebugPrintf("verified = %d\tCensored = %d\n",
                            packet->m_bVerified, packet->m_bCensored);
            ste->SetVerified(packet->m_bVerified);
            ste->SetCensored(packet->m_bCensored);

            ste->setChanged();
        } else {
            app.DebugPrintf(
                "std::dynamic_pointer_cast<SignTileEntity>(te) == nullptr\n");
        }
    } else {
        app.DebugPrintf("hasChunkAt failed\n");
    }
}

void ClientConnection::handleTileEntityData(
    std::shared_ptr<TileEntityDataPacket> packet) {
    if (minecraft->level->hasChunkAt(packet->x, packet->y, packet->z)) {
        std::shared_ptr<TileEntity> te =
            minecraft->level->getTileEntity(packet->x, packet->y, packet->z);

        if (te != nullptr) {
            if (packet->type == TileEntityDataPacket::TYPE_MOB_SPAWNER &&
                std::dynamic_pointer_cast<MobSpawnerTileEntity>(te) !=
                    nullptr) {
                std::dynamic_pointer_cast<MobSpawnerTileEntity>(te)->load(
                    packet->tag);
            } else if (packet->type == TileEntityDataPacket::TYPE_ADV_COMMAND &&
                       std::dynamic_pointer_cast<CommandBlockEntity>(te) !=
                           nullptr) {
                std::dynamic_pointer_cast<CommandBlockEntity>(te)->load(
                    packet->tag);
            } else if (packet->type == TileEntityDataPacket::TYPE_BEACON &&
                       std::dynamic_pointer_cast<BeaconTileEntity>(te) !=
                           nullptr) {
                std::dynamic_pointer_cast<BeaconTileEntity>(te)->load(
                    packet->tag);
            } else if (packet->type == TileEntityDataPacket::TYPE_SKULL &&
                       std::dynamic_pointer_cast<SkullTileEntity>(te) !=
                           nullptr) {
                std::dynamic_pointer_cast<SkullTileEntity>(te)->load(
                    packet->tag);
            }
        }
    }
}

void ClientConnection::handleContainerSetData(
    std::shared_ptr<ContainerSetDataPacket> packet) {
    onUnhandledPacket(packet);
    if (minecraft->localplayers[m_userIndex]->containerMenu != nullptr &&
        minecraft->localplayers[m_userIndex]->containerMenu->containerId ==
            packet->containerId) {
        minecraft->localplayers[m_userIndex]->containerMenu->setData(
            packet->id, packet->value);
    }
}

void ClientConnection::handleSetEquippedItem(
    std::shared_ptr<SetEquippedItemPacket> packet) {
    std::shared_ptr<Entity> entity = getEntity(packet->entity);
    if (entity != nullptr) {
        
        
        
        entity->setEquippedSlot(packet->slot, packet->getItem());
    }
}

void ClientConnection::handleContainerClose(
    std::shared_ptr<ContainerClosePacket> packet) {
    minecraft->localplayers[m_userIndex]->clientSideCloseContainer();
}

void ClientConnection::handleTileEvent(
    std::shared_ptr<TileEventPacket> packet) {
    minecraft->level->tileEvent(packet->x, packet->y, packet->z, packet->tile,
                                packet->b0, packet->b1);
}

void ClientConnection::handleTileDestruction(
    std::shared_ptr<TileDestructionPacket> packet) {
    minecraft->level->destroyTileProgress(packet->getEntityId(), packet->getX(),
                                          packet->getY(), packet->getZ(),
                                          packet->getState());
}

bool ClientConnection::canHandleAsyncPackets() {
    return minecraft != nullptr && minecraft->level != nullptr &&
           minecraft->localplayers[m_userIndex] != nullptr && level != nullptr;
}

void ClientConnection::handleGameEvent(
    std::shared_ptr<GameEventPacket> gameEventPacket) {
    int event = gameEventPacket->_event;
    int param = gameEventPacket->param;
    if (event >= 0 && event < GameEventPacket::EVENT_LANGUAGE_ID_LENGTH) {
        if (GameEventPacket::EVENT_LANGUAGE_ID[event] >
            0)  
        {
            minecraft->localplayers[m_userIndex]->displayClientMessage(
                GameEventPacket::EVENT_LANGUAGE_ID[event]);
        }
    }
    if (event == GameEventPacket::START_RAINING) {
        level->getLevelData()->setRaining(true);
        level->setRainLevel(1);
        if (gameEventPacket->param == 1) {
            level->getLevelData()->setThundering(true);
            level->setThunderLevel(1);
        } else {
            level->getLevelData()->setThundering(false);
            level->setThunderLevel(0);
        }
    } else if (event == GameEventPacket::STOP_RAINING) {
        level->getLevelData()->setRaining(false);
        level->setRainLevel(0);
        level->getLevelData()->setThundering(false);
        level->setThunderLevel(0);
    } else if (event == GameEventPacket::CHANGE_GAME_MODE) {
        minecraft->localgameModes[m_userIndex]->setLocalMode(
            GameType::byId(param));
    } else if (event == GameEventPacket::WIN_GAME) {
        ui.SetWinUserIndex(static_cast<unsigned int>(gameEventPacket->param));

        app.DebugPrintf("handleGameEvent packet for WIN_GAME - %d\n",
                        m_userIndex);
        
        if (minecraft->localgameModes[InputManager.GetPrimaryPad()] != nullptr)
            minecraft->localgameModes[InputManager.GetPrimaryPad()]
                ->getTutorial()
                ->showTutorialPopup(false);
        ui.NavigateToScene(InputManager.GetPrimaryPad(), eUIScene_EndPoem,
                           nullptr, eUILayer_Scene, eUIGroup_Fullscreen);
    } else if (event == GameEventPacket::START_SAVING) {
        if (!g_NetworkManager.IsHost()) {
            
            
            
            app.SetGameStarted(false);
            app.SetAction(InputManager.GetPrimaryPad(),
                          eAppAction_RemoteServerSave);
        }
    } else if (event == GameEventPacket::STOP_SAVING) {
        if (!g_NetworkManager.IsHost()) app.SetGameStarted(true);
    } else if (event == GameEventPacket::SUCCESSFUL_BOW_HIT) {
        std::shared_ptr<MultiplayerLocalPlayer> player =
            minecraft->localplayers[m_userIndex];
        level->playLocalSound(player->x, player->y + player->getHeadHeight(),
                              player->z, eSoundType_RANDOM_BOW_HIT, 0.18f,
                              0.45f, false);
    }
}

void ClientConnection::handleComplexItemData(
    std::shared_ptr<ComplexItemDataPacket> packet) {
    if (packet->itemType == Item::map->id) {
        MapItem::getSavedData(packet->itemId, minecraft->level)
            ->handleComplexItemData(packet->data);
    } else {
        
        
    }
}

void ClientConnection::handleLevelEvent(
    std::shared_ptr<LevelEventPacket> packet) {
    if (packet->type == LevelEvent::SOUND_DRAGON_DEATH) {
        for (unsigned int i = 0; i < XUSER_MAX_COUNT; ++i) {
            if (minecraft->localplayers[i] != nullptr &&
                minecraft->localplayers[i]->level != nullptr &&
                minecraft->localplayers[i]->level->dimension->id == 1) {
                minecraft->localplayers[i]->awardStat(
                    GenericStats::completeTheEnd(),
                    GenericStats::param_noArgs());
            }
        }
    }

    if (packet->isGlobalEvent()) {
        minecraft->level->globalLevelEvent(packet->type, packet->x, packet->y,
                                           packet->z, packet->data);
    } else {
        minecraft->level->levelEvent(packet->type, packet->x, packet->y,
                                     packet->z, packet->data);
    }

    minecraft->level->levelEvent(packet->type, packet->x, packet->y, packet->z,
                                 packet->data);
}

void ClientConnection::handleAwardStat(
    std::shared_ptr<AwardStatPacket> packet) {
    std::vector<uint8_t> paramData = packet->getParamData();
    minecraft->localplayers[m_userIndex]->awardStatFromServer(
        GenericStats::stat(packet->statId), paramData);
}

void ClientConnection::handleUpdateMobEffect(
    std::shared_ptr<UpdateMobEffectPacket> packet) {
    std::shared_ptr<Entity> e = getEntity(packet->entityId);
    if ((e == nullptr) || !e->instanceof(eTYPE_LIVINGENTITY)) return;

    
    
    

    MobEffectInstance* mobEffectInstance = new MobEffectInstance(
        packet->effectId, packet->effectDurationTicks, packet->effectAmplifier);
    mobEffectInstance->setNoCounter(packet->isSuperLongDuration());
    std::dynamic_pointer_cast<LivingEntity>(e)->addEffect(mobEffectInstance);
}

void ClientConnection::handleRemoveMobEffect(
    std::shared_ptr<RemoveMobEffectPacket> packet) {
    std::shared_ptr<Entity> e = getEntity(packet->entityId);
    if ((e == nullptr) || !e->instanceof(eTYPE_LIVINGENTITY)) return;

    (std::dynamic_pointer_cast<LivingEntity>(e))
        ->removeEffectNoUpdate(packet->effectId);
}

bool ClientConnection::isServerPacketListener() { return false; }

void ClientConnection::handlePlayerInfo(
    std::shared_ptr<PlayerInfoPacket> packet) {
    unsigned int startingPrivileges =
        app.GetPlayerPrivileges(packet->m_networkSmallId);

    INetworkPlayer* networkPlayer =
        g_NetworkManager.GetPlayerBySmallId(packet->m_networkSmallId);

    if (networkPlayer != nullptr && networkPlayer->IsHost()) {
        
        Player::enableAllPlayerPrivileges(startingPrivileges, true);
        Player::setPlayerGamePrivilege(startingPrivileges,
                                       Player::ePlayerGamePrivilege_HOST, 1);
    }

    
    app.UpdatePlayerInfo(packet->m_networkSmallId, packet->m_playerColourIndex,
                         packet->m_playerPrivileges);

    std::shared_ptr<Entity> entity = getEntity(packet->m_entityId);
    if (entity != nullptr && entity->instanceof(eTYPE_PLAYER)) {
        std::shared_ptr<Player> player =
            std::dynamic_pointer_cast<Player>(entity);
        player->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_All,
                                       packet->m_playerPrivileges);
    }
    if (networkPlayer != nullptr && networkPlayer->IsLocal()) {
        for (unsigned int i = 0; i < XUSER_MAX_COUNT; ++i) {
            std::shared_ptr<MultiplayerLocalPlayer> localPlayer =
                minecraft->localplayers[i];
            if (localPlayer != nullptr && localPlayer->connection != nullptr &&
                localPlayer->connection->getNetworkPlayer() == networkPlayer) {
                localPlayer->setPlayerGamePrivilege(
                    Player::ePlayerGamePrivilege_All,
                    packet->m_playerPrivileges);
                displayPrivilegeChanges(localPlayer, startingPrivileges);
                break;
            }
        }
    }

    
    
}

void ClientConnection::displayPrivilegeChanges(
    std::shared_ptr<MultiplayerLocalPlayer> player,
    unsigned int oldPrivileges) {
    int userIndex = player->GetXboxPad();
    unsigned int newPrivileges = player->getAllPlayerGamePrivileges();
    Player::EPlayerGamePrivileges priv = (Player::EPlayerGamePrivileges)0;
    bool privOn = false;
    for (unsigned int i = 0; i < Player::ePlayerGamePrivilege_MAX; ++i) {
        priv = (Player::EPlayerGamePrivileges)i;
        if (Player::getPlayerGamePrivilege(newPrivileges, priv) !=
            Player::getPlayerGamePrivilege(oldPrivileges, priv)) {
            privOn = Player::getPlayerGamePrivilege(newPrivileges, priv);
            std::wstring message = L"";
            if (app.GetGameHostOption(eGameHostOption_TrustPlayers) == 0) {
                switch (priv) {
                    case Player::ePlayerGamePrivilege_CannotMine:
                        if (privOn)
                            message = app.GetString(IDS_PRIV_MINE_TOGGLE_ON);
                        else
                            message = app.GetString(IDS_PRIV_MINE_TOGGLE_OFF);
                        break;
                    case Player::ePlayerGamePrivilege_CannotBuild:
                        if (privOn)
                            message = app.GetString(IDS_PRIV_BUILD_TOGGLE_ON);
                        else
                            message = app.GetString(IDS_PRIV_BUILD_TOGGLE_OFF);
                        break;
                    case Player::ePlayerGamePrivilege_CanUseDoorsAndSwitches:
                        if (privOn)
                            message =
                                app.GetString(IDS_PRIV_USE_DOORS_TOGGLE_ON);
                        else
                            message =
                                app.GetString(IDS_PRIV_USE_DOORS_TOGGLE_OFF);
                        break;
                    case Player::ePlayerGamePrivilege_CanUseContainers:
                        if (privOn)
                            message = app.GetString(
                                IDS_PRIV_USE_CONTAINERS_TOGGLE_ON);
                        else
                            message = app.GetString(
                                IDS_PRIV_USE_CONTAINERS_TOGGLE_OFF);
                        break;
                    case Player::ePlayerGamePrivilege_CannotAttackAnimals:
                        if (privOn)
                            message =
                                app.GetString(IDS_PRIV_ATTACK_ANIMAL_TOGGLE_ON);
                        else
                            message = app.GetString(
                                IDS_PRIV_ATTACK_ANIMAL_TOGGLE_OFF);
                        break;
                    case Player::ePlayerGamePrivilege_CannotAttackMobs:
                        if (privOn)
                            message =
                                app.GetString(IDS_PRIV_ATTACK_MOB_TOGGLE_ON);
                        else
                            message =
                                app.GetString(IDS_PRIV_ATTACK_MOB_TOGGLE_OFF);
                        break;
                    case Player::ePlayerGamePrivilege_CannotAttackPlayers:
                        if (privOn)
                            message =
                                app.GetString(IDS_PRIV_ATTACK_PLAYER_TOGGLE_ON);
                        else
                            message = app.GetString(
                                IDS_PRIV_ATTACK_PLAYER_TOGGLE_OFF);
                        break;
                    default:
                        break;
                };
            }
            switch (priv) {
                case Player::ePlayerGamePrivilege_Op:
                    if (privOn)
                        message = app.GetString(IDS_PRIV_MODERATOR_TOGGLE_ON);
                    else
                        message = app.GetString(IDS_PRIV_MODERATOR_TOGGLE_OFF);
                    break;
                default:
                    break;
            };
            if (app.GetGameHostOption(eGameHostOption_CheatsEnabled) != 0) {
                switch (priv) {
                    case Player::ePlayerGamePrivilege_CanFly:
                        if (privOn)
                            message = app.GetString(IDS_PRIV_FLY_TOGGLE_ON);
                        else
                            message = app.GetString(IDS_PRIV_FLY_TOGGLE_OFF);
                        break;
                    case Player::ePlayerGamePrivilege_ClassicHunger:
                        if (privOn)
                            message =
                                app.GetString(IDS_PRIV_EXHAUSTION_TOGGLE_ON);
                        else
                            message =
                                app.GetString(IDS_PRIV_EXHAUSTION_TOGGLE_OFF);
                        break;
                    case Player::ePlayerGamePrivilege_Invisible:
                        if (privOn)
                            message =
                                app.GetString(IDS_PRIV_INVISIBLE_TOGGLE_ON);
                        else
                            message =
                                app.GetString(IDS_PRIV_INVISIBLE_TOGGLE_OFF);
                        break;
                    case Player::ePlayerGamePrivilege_Invulnerable:
                        if (privOn)
                            message =
                                app.GetString(IDS_PRIV_INVULNERABLE_TOGGLE_ON);
                        else
                            message =
                                app.GetString(IDS_PRIV_INVULNERABLE_TOGGLE_OFF);
                        break;
                    case Player::ePlayerGamePrivilege_CanToggleInvisible:
                        if (privOn)
                            message =
                                app.GetString(IDS_PRIV_CAN_INVISIBLE_TOGGLE_ON);
                        else
                            message = app.GetString(
                                IDS_PRIV_CAN_INVISIBLE_TOGGLE_OFF);
                        break;
                    case Player::ePlayerGamePrivilege_CanToggleFly:
                        if (privOn)
                            message = app.GetString(IDS_PRIV_CAN_FLY_TOGGLE_ON);
                        else
                            message =
                                app.GetString(IDS_PRIV_CAN_FLY_TOGGLE_OFF);
                        break;
                    case Player::ePlayerGamePrivilege_CanToggleClassicHunger:
                        if (privOn)
                            message = app.GetString(
                                IDS_PRIV_CAN_EXHAUSTION_TOGGLE_ON);
                        else
                            message = app.GetString(
                                IDS_PRIV_CAN_EXHAUSTION_TOGGLE_OFF);
                        break;
                    case Player::ePlayerGamePrivilege_CanTeleport:
                        if (privOn)
                            message =
                                app.GetString(IDS_PRIV_CAN_TELEPORT_TOGGLE_ON);
                        else
                            message =
                                app.GetString(IDS_PRIV_CAN_TELEPORT_TOGGLE_OFF);
                        break;
                    default:
                        break;
                };
            }
            if (!message.empty())
                minecraft->gui->addMessage(message, userIndex);
        }
    }
}

void ClientConnection::handleKeepAlive(
    std::shared_ptr<KeepAlivePacket> packet) {
    send(std::make_shared<KeepAlivePacket>(packet->id));
}

void ClientConnection::handlePlayerAbilities(
    std::shared_ptr<PlayerAbilitiesPacket> playerAbilitiesPacket) {
    std::shared_ptr<Player> player = minecraft->localplayers[m_userIndex];
    player->abilities.flying = playerAbilitiesPacket->isFlying();
    player->abilities.instabuild = playerAbilitiesPacket->canInstabuild();
    player->abilities.invulnerable = playerAbilitiesPacket->isInvulnerable();
    player->abilities.mayfly = playerAbilitiesPacket->canFly();
    player->abilities.setFlyingSpeed(playerAbilitiesPacket->getFlyingSpeed());
    player->abilities.setWalkingSpeed(playerAbilitiesPacket->getWalkingSpeed());
}

void ClientConnection::handleSoundEvent(
    std::shared_ptr<LevelSoundPacket> packet) {
    minecraft->level->playLocalSound(
        packet->getX(), packet->getY(), packet->getZ(), packet->getSound(),
        packet->getVolume(), packet->getPitch(), false);
}

void ClientConnection::handleCustomPayload(
    std::shared_ptr<CustomPayloadPacket> customPayloadPacket) {
    if (CustomPayloadPacket::TRADER_LIST_PACKET.compare(
            customPayloadPacket->identifier) == 0) {
        ByteArrayInputStream bais(customPayloadPacket->data);
        DataInputStream input(&bais);
        int containerId = input.readInt();
#ifdef ENABLE_JAVA_GUIS
        
        
        if (minecraft->screen &&
            dynamic_cast<MerchantScreen*>(minecraft->screen) &&
            containerId == minecraft->localplayers[m_userIndex]
                               ->containerMenu->containerId) {
            std::shared_ptr<Merchant> trader = nullptr;
            MerchantScreen* screen = (MerchantScreen*)minecraft->screen;
            trader = screen->getMerchant();
#else
        if (ui.IsSceneInStack(m_userIndex, eUIScene_TradingMenu) &&
            containerId == minecraft->localplayers[m_userIndex]
                               ->containerMenu->containerId) {
            std::shared_ptr<Merchant> trader = nullptr;

            UIScene* scene = ui.GetTopScene(m_userIndex, eUILayer_Scene);
            UIScene_TradingMenu* screen = (UIScene_TradingMenu*)scene;
            trader = screen->getMerchant();
#endif
            MerchantRecipeList* recipeList =
                MerchantRecipeList::createFromStream(&input);
            trader->overrideOffers(recipeList);
        }
    }
}

Connection* ClientConnection::getConnection() { return connection; }




void ClientConnection::handleJavaTabList(
    std::shared_ptr<JavaTabListPacket> packet) {
    if (minecraft == nullptr) return;
    switch (packet->action) {
        case JavaTabListPacket::ACTION_ADD:
            minecraft->javaTabAdd(packet->uuid, packet->name, packet->ping);
            break;
        case JavaTabListPacket::ACTION_REMOVE:
            minecraft->javaTabRemove(packet->uuid);
            break;
        case JavaTabListPacket::ACTION_CLEAR:
            minecraft->javaTabClear();
            break;
        case JavaTabListPacket::ACTION_BIND: {
            if (level == nullptr) break;
            std::shared_ptr<Entity> e = level->getEntity(packet->entityId);
            std::shared_ptr<Player> p = std::dynamic_pointer_cast<Player>(e);
            if (p != nullptr) p->setJavaUuid(packet->uuid);
            break;
        }
    }
}

void ClientConnection::handleServerSettingsChanged(
    std::shared_ptr<ServerSettingsChangedPacket> packet) {
    if (packet->action == ServerSettingsChangedPacket::HOST_IN_GAME_SETTINGS) {
        app.SetGameHostOption(eGameHostOption_All, packet->data);
    } else if (packet->action == ServerSettingsChangedPacket::HOST_DIFFICULTY) {
        for (unsigned int i = 0; i < minecraft->levels.size(); ++i) {
            if (minecraft->levels[i] != nullptr) {
                app.DebugPrintf(
                    "ClientConnection::handleServerSettingsChanged - "
                    "Difficulty = %d",
                    packet->data);
                minecraft->levels[i]->difficulty = packet->data;
            }
        }
    } else {
        
        
        app.SetGameHostOption(eGameHostOption_Gamertags, packet->data);
    }
}

void ClientConnection::handleXZ(std::shared_ptr<XZPacket> packet) {
    if (packet->action == XZPacket::STRONGHOLD) {
        minecraft->levels[0]->getLevelData()->setXStronghold(packet->x);
        minecraft->levels[0]->getLevelData()->setZStronghold(packet->z);
        minecraft->levels[0]->getLevelData()->setHasStronghold();
    }
}

void ClientConnection::handleUpdateProgress(
    std::shared_ptr<UpdateProgressPacket> packet) {
    if (!g_NetworkManager.IsHost())
        Minecraft::GetInstance()->progressRenderer->progressStagePercentage(
            packet->m_percentage);
}

void ClientConnection::handleUpdateGameRuleProgressPacket(
    std::shared_ptr<UpdateGameRuleProgressPacket> packet) {
    const wchar_t* string = app.GetGameRulesString(packet->m_messageId);
    if (string != nullptr) {
        std::wstring message(string);
        message = GameRuleDefinition::generateDescriptionString(
            packet->m_definitionType, message, packet->m_data.data(),
            packet->m_data.size());
        if (minecraft->localgameModes[m_userIndex] != nullptr) {
            minecraft->localgameModes[m_userIndex]->getTutorial()->setMessage(
                message, packet->m_icon, packet->m_auxValue);
        }
    }
    
    
    if (packet->m_dataTag > 0 && packet->m_dataTag <= 32) {
        app.DebugPrintf(
            "handleUpdateGameRuleProgressPacket: Data tag is in range, so "
            "updating profile data\n");
        app.SetSpecialTutorialCompletionFlag(m_userIndex,
                                             packet->m_dataTag - 1);
    }
}




int ClientConnection::HostDisconnectReturned(
    void* pParam, int iPad, C4JStorage::EMessageResult result) {
    
    
    if (!Minecraft::GetInstance()->skins->isUsingDefaultSkin()) {
        TexturePack* tPack = Minecraft::GetInstance()->skins->getSelected();
        DLCTexturePack* pDLCTexPack = (DLCTexturePack*)tPack;

        DLCPack* pDLCPack =
            pDLCTexPack->getDLCInfoParentPack();  
        if (!pDLCPack->hasPurchasedFile(DLCManager::e_DLCType_Texture, L"")) {
            
            MinecraftServer::getInstance()->setSaveOnExit(false);
            
            app.SetAction(iPad, eAppAction_ExitWorld);
        }
    }

    
    
    bool bSaveExists;
    StorageManager.DoesSaveExist(&bSaveExists);
    
    
    if (bSaveExists) {
        unsigned int uiIDA[2];
        uiIDA[0] = IDS_CONFIRM_CANCEL;
        uiIDA[1] = IDS_CONFIRM_OK;
        ui.RequestErrorMessage(IDS_TITLE_SAVE_GAME, IDS_CONFIRM_SAVE_GAME,
                               uiIDA, 2, InputManager.GetPrimaryPad(),
                               &ClientConnection::ExitGameAndSaveReturned,
                               nullptr);
    } else {
        MinecraftServer::getInstance()->setSaveOnExit(true);
        
        app.SetAction(iPad, eAppAction_ExitWorld);
    }

    return 0;
}

int ClientConnection::ExitGameAndSaveReturned(
    void* pParam, int iPad, C4JStorage::EMessageResult result) {
    
    if (result == C4JStorage::EMessage_ResultDecline) {
        
        
        
        
        
        MinecraftServer::getInstance()->setSaveOnExit(true);
    } else {
        MinecraftServer::getInstance()->setSaveOnExit(false);
    }
    
    app.SetAction(iPad, eAppAction_ExitWorld);
    return 0;
}


std::wstring ClientConnection::GetDisplayNameByGamertag(std::wstring gamertag) {
    return gamertag;
}

void ClientConnection::handleAddObjective(
    std::shared_ptr<SetObjectivePacket> packet) {
    if (level == nullptr || level->getScoreboard() == nullptr) return;
    Scoreboard* scoreboard = level->getScoreboard();
    fprintf(stderr, "[JSCORE] client objective method=%d name='%ls'\n",
            packet->method, packet->objectiveName.c_str());
    if (packet->method == 0) {
        Objective* objective =
            scoreboard->addObjective(packet->objectiveName, nullptr);
        objective->setDisplayName(packet->displayName);
    } else {
        Objective* objective = scoreboard->getObjective(packet->objectiveName);
        if (objective == nullptr) return;
        if (packet->method == 1) {
            scoreboard->removeObjective(objective);
        } else if (packet->method == 2) {
            objective->setDisplayName(packet->displayName);
        }
    }
}

void ClientConnection::handleSetScore(std::shared_ptr<SetScorePacket> packet) {
    if (level == nullptr || level->getScoreboard() == nullptr) return;
    Scoreboard* scoreboard = level->getScoreboard();
    fprintf(stderr, "[JSCORE] client score method=%d owner='%ls' value=%d\n",
            packet->method, packet->owner.c_str(), packet->score);
    if (packet->method == SetScorePacket::METHOD_REMOVE) {
        scoreboard->resetPlayerScore(packet->owner);
        return;
    }
    Objective* objective = scoreboard->getObjective(packet->objectiveName);
    if (objective == nullptr) return;
    scoreboard->getPlayerScore(packet->owner, objective)
        ->setScore(packet->score);
}

void ClientConnection::handleSetDisplayObjective(
    std::shared_ptr<SetDisplayObjectivePacket> packet) {
    if (level == nullptr || level->getScoreboard() == nullptr) return;
    Scoreboard* scoreboard = level->getScoreboard();
    fprintf(stderr, "[JSCORE] client display slot=%d name='%ls'\n",
            packet->slot, packet->objectiveName.c_str());
    if (packet->objectiveName.empty()) {
        scoreboard->setDisplayObjective(packet->slot, nullptr);
    } else {
        scoreboard->setDisplayObjective(
            packet->slot, scoreboard->getObjective(packet->objectiveName));
    }
}

void ClientConnection::handleSetPlayerTeamPacket(
    std::shared_ptr<SetPlayerTeamPacket> packet) {
    if (level == nullptr || level->getScoreboard() == nullptr) return;
    Scoreboard* scoreboard = level->getScoreboard();
    fprintf(stderr, "[JSCORE] client team method=%d name='%ls' players=%zu\n",
            packet->method, packet->name.c_str(), packet->players.size());

    if (packet->method == SetPlayerTeamPacket::METHOD_ADD) {
        PlayerTeam* team = scoreboard->addPlayerTeam(packet->name);
        team->setDisplayName(packet->displayName);
        team->setPrefix(packet->prefix);
        team->setSuffix(packet->suffix);
        team->unpackOptions(packet->options);
        for (const std::wstring& player : packet->players)
            scoreboard->addPlayerToTeam(player, team);
        return;
    }

    PlayerTeam* team = scoreboard->getPlayerTeam(packet->name);
    if (team == nullptr) return;
    switch (packet->method) {
        case SetPlayerTeamPacket::METHOD_REMOVE:
            scoreboard->removePlayerTeam(team);
            break;
        case SetPlayerTeamPacket::METHOD_CHANGE:
            team->setDisplayName(packet->displayName);
            team->setPrefix(packet->prefix);
            team->setSuffix(packet->suffix);
            team->unpackOptions(packet->options);
            break;
        case SetPlayerTeamPacket::METHOD_JOIN:
            for (const std::wstring& player : packet->players)
                scoreboard->addPlayerToTeam(player, team);
            break;
        case SetPlayerTeamPacket::METHOD_LEAVE:
            for (const std::wstring& player : packet->players)
                scoreboard->removePlayerFromTeam(player, team);
            break;
    }
}

void ClientConnection::handleHudOverlay(
    std::shared_ptr<HudOverlayPacket> packet) {
    Gui* gui = minecraft != nullptr ? minecraft->gui : nullptr;
    if (gui == nullptr) return;
    switch (packet->action) {
        case HudOverlayPacket::ACTION_ACTIONBAR:
            gui->setActionBar(packet->text);
            break;
        case HudOverlayPacket::ACTION_TITLE:
            gui->setTitle(packet->text);
            break;
        case HudOverlayPacket::ACTION_SUBTITLE:
            gui->setSubtitle(packet->text);
            break;
        case HudOverlayPacket::ACTION_TIMES:
            gui->setTitleTimes(packet->fadeIn, packet->stay, packet->fadeOut);
            break;
        case HudOverlayPacket::ACTION_CLEAR:
            gui->clearTitles(false);
            break;
        case HudOverlayPacket::ACTION_RESET:
            gui->clearTitles(true);
            break;
    }
}

void ClientConnection::handleParticleEvent(
    std::shared_ptr<LevelParticlesPacket> packet) {
    for (int i = 0; i < packet->getCount(); i++) {
        double xVarience = random->nextGaussian() * packet->getXDist();
        double yVarience = random->nextGaussian() * packet->getYDist();
        double zVarience = random->nextGaussian() * packet->getZDist();
        double xa = random->nextGaussian() * packet->getMaxSpeed();
        double ya = random->nextGaussian() * packet->getMaxSpeed();
        double za = random->nextGaussian() * packet->getMaxSpeed();

        
        assert(0);
        ePARTICLE_TYPE particleId = eParticleType_heart;

        level->addParticle(particleId, packet->getX() + xVarience,
                           packet->getY() + yVarience,
                           packet->getZ() + zVarience, xa, ya, za);
    }
}

void ClientConnection::handleUpdateAttributes(
    std::shared_ptr<UpdateAttributesPacket> packet) {
    std::shared_ptr<Entity> entity = getEntity(packet->getEntityId());
    if (entity == nullptr) return;

    if (!entity->instanceof(eTYPE_LIVINGENTITY)) {
        
        assert(0);
    }

    BaseAttributeMap* attributes =
        (std::dynamic_pointer_cast<LivingEntity>(entity))->getAttributes();
    std::unordered_set<UpdateAttributesPacket::AttributeSnapshot*>
        attributeSnapshots = packet->getValues();
    for (auto it = attributeSnapshots.begin(); it != attributeSnapshots.end();
         ++it) {
        UpdateAttributesPacket::AttributeSnapshot* attribute = *it;
        AttributeInstance* instance =
            attributes->getInstance(attribute->getId());

        if (instance == nullptr) {
            
            
            
            instance = attributes->registerAttribute(new RangedAttribute(
                attribute->getId(), 0, std::numeric_limits<double>::min(),
                std::numeric_limits<double>::max()));
        }

        instance->setBaseValue(attribute->getBase());
        instance->removeModifiers();

        std::unordered_set<AttributeModifier*>* modifiers =
            attribute->getModifiers();

        for (auto it2 = modifiers->begin(); it2 != modifiers->end(); ++it2) {
            AttributeModifier* modifier = *it2;
            instance->addModifier(
                new AttributeModifier(modifier->getId(), modifier->getAmount(),
                                      modifier->getOperation()));
        }
    }
}



void ClientConnection::checkDeferredEntityLinkPackets(int newEntityId) {
    if (deferredEntityLinkPackets.empty()) return;

    for (int i = 0; i < deferredEntityLinkPackets.size(); i++) {
        DeferredEntityLinkPacket* deferred = &deferredEntityLinkPackets[i];

        bool remove = false;

        
        auto tickInterval =
            std::chrono::duration_cast<std::chrono::milliseconds>(time_util::clock::now() - deferred->m_recievedTick).count();
        if (tickInterval < MAX_ENTITY_LINK_DEFERRAL_INTERVAL) {
            
            if (deferred->m_packet->destId == newEntityId) {
                handleEntityLinkPacket(deferred->m_packet);
                remove = true;
            }
        } else {
            
            
            remove = true;
        }

        if (remove) {
            deferredEntityLinkPackets.erase(deferredEntityLinkPackets.begin() +
                                            i);
            i--;
        }
    }
}

ClientConnection::DeferredEntityLinkPacket::DeferredEntityLinkPacket(
    std::shared_ptr<SetEntityLinkPacket> packet) {
    m_recievedTick = time_util::clock::now();
    m_packet = packet;
}
