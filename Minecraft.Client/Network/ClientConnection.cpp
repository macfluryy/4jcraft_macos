#include "../Platform/stdafx.h"
#include "ClientConnection.h"
#include <cfloat>
#include <limits>
#include "../Level/MultiPlayerLevel.h"
#include "../Player/MultiPlayerLocalPlayer.h"
#include "../GameState/StatsCounter.h"
#include "../UI/Screens/ReceivingLevelScreen.h"
#include "../Player/RemotePlayer.h"
#include "../UI/Screens/DisconnectedScreen.h"
#include "../Rendering/Particles/TakeAnimationParticle.h"
#include "../Rendering/Particles/CritParticle.h"
#include "../Player/User.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.level.storage.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.level.chunk.h"
#include "../../Minecraft.World/Headers/net.minecraft.stats.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.entity.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.entity.ai.attributes.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.entity.player.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.entity.animal.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.entity.npc.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.entity.item.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.entity.projectile.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.entity.global.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.entity.boss.enderdragon.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.entity.monster.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.level.tile.entity.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.item.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.item.trading.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.level.tile.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.inventory.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.level.saveddata.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.level.dimension.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.effect.h"
#include "../../Minecraft.World/Headers/net.minecraft.world.food.h"
#include "../../Minecraft.World/Util/SharedConstants.h"
#include "../../Minecraft.World/Util/AABB.h"
#include "../../Minecraft.World/Util/Pos.h"
#include "../../Minecraft.World/Network/Socket.h"
#include "../Minecraft.h"
#include "../Rendering/EntityRenderers/ProgressRenderer.h"
#include "../Rendering/LevelRenderer.h"
#include "../GameState/Options.h"
#include "../MinecraftServer.h"
#include "../ClientConstants.h"
#include "../../Minecraft.World/Util/SoundTypes.h"
#include "../Textures/Packs/TexturePackRepository.h"
#include "UI/Screens/MerchantScreen.h"
#include "../Platform/Common/UI/UI.h"
#include "../Textures/Packs/DLCTexturePack.h"

ClientConnection::ClientConnection(Minecraft* minecraft, const std::wstring& ip,
                                   int port) {
    // 4J Stu - No longer used as we use the socket version below.
    assert(false);
}

ClientConnection::ClientConnection(Minecraft* minecraft, Socket* socket,
                                   int iUserIndex /*= -1*/) {
    // 4J - added initiliasers
    random = new Random();
    done = false;
    level = nullptr;
    started = false;
    savedDataStorage = new SavedDataStorage(nullptr);
    maxPlayers = 20;

    this->minecraft = minecraft;

    if (iUserIndex < 0) {
        m_userIndex = ProfileManager.GetPrimaryPad();
    } else {
        m_userIndex = iUserIndex;
    }

    if (socket == nullptr) {
        socket = new Socket();  // 4J - Local connection
    }

    createdOk = socket->createdOk;
    if (createdOk) {
        connection = new Connection(socket, L"Client", this);
    } else {
        connection = nullptr;
        // TODO 4J Stu - This will cause issues since the session player owns
        // the socket
        // delete socket;
    }

    deferredEntityLinkPackets = std::vector<DeferredEntityLinkPacket>();
}

ClientConnection::~ClientConnection() {
    delete connection;
    delete random;
    delete savedDataStorage;
}

void ClientConnection::tick() {
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
    ProfileManager.GetXUID(m_userIndex, &OnlineXuid, true);  // online xuid
    MOJANG_DATA* pMojangData = nullptr;

    if (!g_NetworkManager.IsLocalGame()) {
        pMojangData = app.GetMojangDataForXuid(OnlineXuid);
    }

    if (!g_NetworkManager.IsHost()) {
        Minecraft::GetInstance()->progressRenderer->progressStagePercentage(
            (eCCLoginReceived * 100) / (eCCConnected));
    }

    // 4J-PB - load the local player skin (from the global title user storage
    // area) if there is one the primary player on the host machine won't have a
    // qnet player from the socket
    INetworkPlayer* networkPlayer = connection->getSocket()->getPlayer();
    int iUserID = -1;

    if (m_userIndex == ProfileManager.GetPrimaryPad()) {
        iUserID = m_userIndex;

        TelemetryManager->SetMultiplayerInstanceId(
            packet->m_multiplayerInstanceId);
    } else {
        if (!networkPlayer->IsGuest() && networkPlayer->IsLocal()) {
            // find the pad number of this local player
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

        // if there's a special skin or cloak for this player, add it in
        if (pMojangData) {
            // a skin?
            if (pMojangData->wchSkin[0] != 0L) {
                std::wstring wstr = pMojangData->wchSkin;
                // check the file is not already in
                bRes = app.IsFileInMemoryTextures(wstr);
                if (!bRes) {
                }

                if (bRes) {
                    app.AddMemoryTextureFile(wstr, pBuffer, dwSize);
                }
            }

            // a cloak?
            if (pMojangData->wchCape[0] != 0L) {
                std::wstring wstr = pMojangData->wchCape;
                // check the file is not already in
                bRes = app.IsFileInMemoryTextures(wstr);
                if (!bRes) {
                }

                if (bRes) {
                    app.AddMemoryTextureFile(wstr, pBuffer, dwSize);
                }
            }
        }

        // If we're online, read the banned game list
        app.ReadBannedList(iUserID);
        // mark the level as not checked against banned levels - it'll be
        // checked once the level starts
        app.SetBanListCheck(iUserID, false);
    }

    if (m_userIndex == ProfileManager.GetPrimaryPad()) {
        if (app.GetTutorialMode()) {
            minecraft->gameMode = new FullTutorialMode(
                ProfileManager.GetPrimaryPad(), minecraft, this);
        }
        // check if we're in the trial version
        else if (ProfileManager.IsFullVersion() == false) {
            minecraft->gameMode =
                new TrialMode(ProfileManager.GetPrimaryPad(), minecraft, this);
        } else {
            MemSect(13);
            minecraft->gameMode = new ConsoleGameMode(
                ProfileManager.GetPrimaryPad(), minecraft, this);
            MemSect(0);
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

            // 4J Stu - We want to share the SavedDataStorage between levels
            int otherDimensionId = packet->dimension == 0 ? -1 : 0;
            Level* activeLevel = minecraft->getLevel(otherDimensionId);
            if (activeLevel != nullptr) {
                // Don't need to delete it here as it belongs to a client
                // connection while will delete it when it's done
                // if( level->savedDataStorage != nullptr ) delete
                // level->savedDataStorage;
                level->savedDataStorage = activeLevel->savedDataStorage;
            }

            app.DebugPrintf("ClientConnection - DIFFICULTY --- %d\n",
                            packet->difficulty);
            level->difficulty = packet->difficulty;  // 4J Added
            level->isClientSide = true;
            minecraft->setLevel(level);
        }

        minecraft->player->setPlayerIndex(packet->m_playerIndex);
        minecraft->player->setCustomSkin(app.GetPlayerSkinId(m_userIndex));
        minecraft->player->setCustomCape(app.GetPlayerCapeId(m_userIndex));

        minecraft->createPrimaryLocalPlayer(ProfileManager.GetPrimaryPad());

        minecraft->player->dimension = packet->dimension;
        minecraft->setScreen(new ReceivingLevelScreen(this));
        minecraft->player->entityId = packet->clientVersion;

        std::uint8_t networkSmallId = getSocket()->getSmallId();
        app.UpdatePlayerInfo(networkSmallId, packet->m_playerIndex,
                             packet->m_uiGamePrivileges);
        minecraft->player->setPlayerGamePrivilege(
            Player::ePlayerGamePrivilege_All, packet->m_uiGamePrivileges);

        // Assume all privileges are on, so that the first message we see only
        // indicates things that have been turned off
        unsigned int startingPrivileges = 0;
        Player::enableAllPlayerPrivileges(startingPrivileges, true);

        if (networkPlayer->IsHost()) {
            Player::setPlayerGamePrivilege(
                startingPrivileges, Player::ePlayerGamePrivilege_HOST, 1);
        }

        displayPrivilegeChanges(minecraft->player, startingPrivileges);

        // update the debugoptions
        app.SetGameSettingsDebugMask(ProfileManager.GetPrimaryPad(),
                                     app.GetGameSettingsDebugMask(-1, true));
    } else {
        // 4J-PB - this isn't the level we want
        // level = (MultiPlayerLevel *)minecraft->level;
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

            dimensionLevel->difficulty = packet->difficulty;  // 4J Added
            dimensionLevel->isClientSide = true;
            level = dimensionLevel;
            // 4J Stu - At time of writing ProfileManager.GetGamertag() does not
            // always return the correct name, if sign-ins are turned off while
            // the player signed in. Using the qnetPlayer instead. need to have
            // a level before create extra local player
            MultiPlayerLevel* levelpassedin = (MultiPlayerLevel*)level;
            player = minecraft->createExtraLocalPlayer(
                m_userIndex, networkPlayer->GetOnlineName(), m_userIndex,
                packet->dimension, this, levelpassedin);

            // need to have a player before the setlevel
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

        // level->addClientConnection( this );
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

        // Assume all privileges are on, so that the first message we see only
        // indicates things that have been turned off
        unsigned int startingPrivileges = 0;
        Player::enableAllPlayerPrivileges(startingPrivileges, true);

        displayPrivilegeChanges(minecraft->localplayers[m_userIndex],
                                startingPrivileges);
    }

    maxPlayers = packet->maxPlayers;

    // need to have a player before the setLocalCreativeMode
    std::shared_ptr<MultiplayerLocalPlayer> lastPlayer = minecraft->player;
    minecraft->player = minecraft->localplayers[m_userIndex];
    ((MultiPlayerGameMode*)minecraft->localgameModes[m_userIndex])
        ->setLocalMode(GameType::byId(packet->gameType));
    minecraft->player = lastPlayer;

    // make sure the UI offsets for this player are set correctly
    if (iUserID != -1) {
        ui.UpdateSelectedItemPos(iUserID);
    }

    TelemetryManager->RecordLevelStart(
        m_userIndex, eSen_FriendOrMatch_Playing_With_Invited_Friends,
        eSen_CompeteOrCoop_Coop_and_Competitive,
        Minecraft::GetInstance()->getLevel(packet->dimension)->difficulty,
        app.GetLocalPlayerCount(), g_NetworkManager.GetOnlinePlayerCount());
}

void ClientConnection::handleAddEntity(
    std::shared_ptr<AddEntityPacket> packet) {
    double x = packet->x / 32.0;
    double y = packet->y / 32.0;
    double z = packet->z / 32.0;
    std::shared_ptr<Entity> e;
    bool setRot = true;

    // 4J-PB - replacing this massive if nest with switch
    switch (packet->type) {
        case AddEntityPacket::MINECART:
            e = Minecart::createMinecart(level, x, y, z, packet->data);
            break;
        case AddEntityPacket::FISH_HOOK: {
            // 4J Stu - Brought forward from 1.4 to be able to drop XP from
            // fishing
            std::shared_ptr<Entity> owner = getEntity(packet->data);

            // 4J - check all local players to find match
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
                // 4J Stu - Move the player->fishing out of the ctor as we
                // cannot reference 'this'
                player->fishing = hook;
            }
            packet->data = 0;
        } break;
        case AddEntityPacket::ARROW:
            e = std::shared_ptr<Entity>(new Arrow(level, x, y, z));
            break;
        case AddEntityPacket::SNOWBALL:
            e = std::shared_ptr<Entity>(new Snowball(level, x, y, z));
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
            e = std::shared_ptr<Entity>(new ThrownEnderpearl(level, x, y, z));
            break;
        case AddEntityPacket::EYEOFENDERSIGNAL:
            e = std::shared_ptr<Entity>(new EyeOfEnderSignal(level, x, y, z));
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
            e = std::shared_ptr<Entity>(new ThrownEgg(level, x, y, z));
            break;
        case AddEntityPacket::THROWN_POTION:
            e = std::shared_ptr<Entity>(
                new ThrownPotion(level, x, y, z, packet->data));
            packet->data = 0;
            break;
        case AddEntityPacket::THROWN_EXPBOTTLE:
            e = std::shared_ptr<Entity>(new ThrownExpBottle(level, x, y, z));
            packet->data = 0;
            break;
        case AddEntityPacket::BOAT:
            e = std::shared_ptr<Entity>(new Boat(level, x, y, z));
            break;
        case AddEntityPacket::PRIMED_TNT:
            e = std::shared_ptr<Entity>(new PrimedTnt(level, x, y, z, nullptr));
            break;
        case AddEntityPacket::ENDER_CRYSTAL:
            e = std::shared_ptr<Entity>(new EnderCrystal(level, x, y, z));
            break;
        case AddEntityPacket::ITEM:
            e = std::shared_ptr<Entity>(new ItemEntity(level, x, y, z));
            break;
        case AddEntityPacket::FALLING:
            e = std::shared_ptr<Entity>(new FallingTile(
                level, x, y, z, packet->data & 0xFFFF, packet->data >> 16));
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
#if !defined(_FINAL_BUILD)
        default:
            // Not a known entity (?)
            assert(0);
#endif
    }

    /*   if (packet->type == AddEntityPacket::MINECART_RIDEABLE) e =
       std::shared_ptr<Entity>( new Minecart(level, x, y, z, Minecart::RIDEABLE)
       ); if (packet->type == AddEntityPacket::MINECART_CHEST) e =
       std::shared_ptr<Entity>( new Minecart(level, x, y, z, Minecart::CHEST) );
       if (packet->type == AddEntityPacket::MINECART_FURNACE) e =
       std::shared_ptr<Entity>( new Minecart(level, x, y, z, Minecart::FURNACE)
       ); if (packet->type == AddEntityPacket::FISH_HOOK)
           {
                   // 4J Stu - Brought forward from 1.4 to be able to drop XP
       from fishing std::shared_ptr<Entity> owner = getEntity(packet->data);

                   // 4J - check all local players to find match
                   if( owner == nullptr )
                   {
                           for( int i = 0; i < XUSER_MAX_COUNT; i++ )
                           {
                                   if( minecraft->localplayers[i] )
                                   {
                                           if(
       minecraft->localplayers[i]->entityId == packet->data )
                                           {

                                                   owner =
       minecraft->localplayers[i]; break;
                                           }
                                   }
                           }
                   }
                   std::shared_ptr<Player> player =
       std::dynamic_pointer_cast<Player>(owner); if (player != nullptr)
                   {
                           std::shared_ptr<FishingHook> hook =
       std::shared_ptr<FishingHook>( new FishingHook(level, x, y, z, player) );
                           e = hook;
                           // 4J Stu - Move the player->fishing out of the ctor
       as we cannot reference 'this' player->fishing = hook;
                   }
                   packet->data = 0;
           }

       if (packet->type == AddEntityPacket::ARROW) e = std::shared_ptr<Entity>(
       new Arrow(level, x, y, z) ); if (packet->type ==
       AddEntityPacket::SNOWBALL) e = std::shared_ptr<Entity>( new
       Snowball(level, x, y, z) ); if (packet->type ==
       AddEntityPacket::THROWN_ENDERPEARL) e = std::shared_ptr<Entity>( new
       ThrownEnderpearl(level, x, y, z) ); if (packet->type ==
       AddEntityPacket::EYEOFENDERSIGNAL) e = std::shared_ptr<Entity>( new
       EyeOfEnderSignal(level, x, y, z) ); if (packet->type ==
       AddEntityPacket::FIREBALL)
           {
                   e = shared_ptr<Entity>( new Fireball(level, x, y, z,
       packet->xa / 8000.0, packet->ya / 8000.0, packet->za / 8000.0) );
                   packet->data = 0;
           }
           if (packet->type == AddEntityPacket::SMALL_FIREBALL)
           {
                   e = shared_ptr<Entity>( new SmallFireball(level, x, y, z,
       packet->xa / 8000.0, packet->ya / 8000.0, packet->za / 8000.0) );
                   packet->data = 0;
           }
           if (packet->type == AddEntityPacket::EGG) e = shared_ptr<Entity>( new
       ThrownEgg(level, x, y, z) ); if (packet->type ==
       AddEntityPacket::THROWN_POTION)
           {
                   e = shared_ptr<Entity>( new ThrownPotion(level, x, y, z,
       packet->data) ); packet->data = 0;
           }
           if (packet->type == AddEntityPacket::THROWN_EXPBOTTLE)
           {
                   e = shared_ptr<Entity>( new ThrownExpBottle(level, x, y, z)
       ); packet->data = 0;
           }
           if (packet->type == AddEntityPacket::BOAT) e = shared_ptr<Entity>(
       new Boat(level, x, y, z) ); if (packet->type ==
       AddEntityPacket::PRIMED_TNT) e = shared_ptr<Entity>( new PrimedTnt(level,
       x, y, z) ); if (packet->type == AddEntityPacket::ENDER_CRYSTAL) e =
       shared_ptr<Entity>( new EnderCrystal(level, x, y, z) ); if (packet->type
       == AddEntityPacket::FALLING_SAND) e = shared_ptr<Entity>( new
       FallingTile(level, x, y, z, Tile::sand->id) ); if (packet->type ==
       AddEntityPacket::FALLING_GRAVEL) e = shared_ptr<Entity>( new
       FallingTile(level, x, y, z, Tile::gravel->id) ); if (packet->type ==
       AddEntityPacket::FALLING_EGG) e = shared_ptr<Entity>( new
       FallingTile(level, x, y, z, Tile::dragonEgg_Id) );

           */

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
            // for (int i = 0; i < subEntities.length; i++)
            for (auto it = subEntities->begin(); it != subEntities->end();
                 ++it) {
                (*it)->entityId += offs;
                // subEntities[i].entityId += offs;
                // System.out.println(subEntities[i].entityId);
            }
        }

        if (packet->type == AddEntityPacket::LEASH_KNOT) {
            // 4J: "Move" leash knot to it's current position, this sets old
            // position (like frame, leash has adjusted position)
            e->absMoveTo(e->x, e->y, e->z, yRot, xRot);
        } else if (packet->type == AddEntityPacket::ITEM_FRAME) {
            // Not doing this move for frame, as the ctor for these objects does
            // some adjustments on the position based on direction to move the
            // object out slightly from what it is attached to, and this just
            // overwrites it
        } else {
            // For everything else, set position
            e->absMoveTo(x, y, z, yRot, xRot);
        }
        e->entityId = packet->id;
        level->putEntity(packet->id, e);

        if (packet->data > -1)  // 4J - changed "no data" value to be -1, we can
                                // have a valid entity id of 0
        {
            if (packet->type == AddEntityPacket::ARROW) {
                std::shared_ptr<Entity> owner = getEntity(packet->data);

                // 4J - check all local players to find match
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

        // 4J: Check our deferred entity link packets
        checkDeferredEntityLinkPackets(e->entityId);
    }
}

void ClientConnection::handleAddExperienceOrb(
    std::shared_ptr<AddExperienceOrbPacket> packet) {
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
    double x = packet->x / 32.0;
    double y = packet->y / 32.0;
    double z = packet->z / 32.0;
    std::shared_ptr<Entity> e;  // = nullptr;
    if (packet->type == AddGlobalEntityPacket::LIGHTNING)
        e = std::shared_ptr<LightningBolt>(new LightningBolt(level, x, y, z));
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
    std::shared_ptr<Painting> painting = std::shared_ptr<Painting>(new Painting(
        level, packet->x, packet->y, packet->z, packet->dir, packet->motive));
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
    // Some remote players could actually be local players that are already
    // added
    for (unsigned int idx = 0; idx < XUSER_MAX_COUNT; ++idx) {
        // need to use the XUID here
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
        new RemotePlayer(minecraft->level, packet->name));
    player->xo = player->xOld = player->xp = packet->x;
    player->yo = player->yOld = player->yp = packet->y;
    player->zo = player->zOld = player->zp = packet->z;
    player->xRotp = packet->xRot;
    player->yRotp = packet->yRot;
    player->yHeadRot = packet->yHeadRot * 360 / 256.0f;
    player->setXuid(packet->xuid);

    // On all other platforms display name is just gamertag so don't check with
    // the network manager
    player->m_displayName = player->name;

    //	printf("\t\t\t\t%d: Add player\n",packet->id,packet->yRot);

    int item = packet->carriedItem;
    if (item == 0) {
        player->inventory->items[player->inventory->selected] =
            std::shared_ptr<ItemInstance>();  // nullptr;
    } else {
        player->inventory->items[player->inventory->selected] =
            std::shared_ptr<ItemInstance>(new ItemInstance(item, 1, 0));
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
        // Update the ref count on the memory texture data
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
        // Update the ref count on the memory texture data
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
    // 4J - make sure xRot stays within -90 -> 90 range
    int ixRot = packet->xRot;
    if (ixRot >= 128) ixRot -= 256;
    float yRot = packet->yRot * 360 / 256.0f;
    float xRot = ixRot * 360 / 256.0f;
    e->yRotp = packet->yRot;
    e->xRotp = ixRot;

    //	printf("\t\t\t\t%d: Teleport to %d (lerp to
    //%f)\n",packet->id,packet->yRot,yRot);
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
    // 4J - The original code did not add the 1/64.0f like the teleport above
    // did, which caused minecarts to fall through the ground
    double y = e->yp / 32.0 + 1 / 64.0f;
    double z = e->zp / 32.0;
    // 4J - have changed rotation to be relative here too
    e->yRotp += packet->yRot;
    e->xRotp += packet->xRot;
    float yRot = (e->yRotp * 360) / 256.0f;
    float xRot = (e->xRotp * 360) / 256.0f;
    //    float yRot = packet->hasRot ? packet->yRot * 360 / 256.0f : e->yRot;
    //    float xRot = packet->hasRot ? packet->xRot * 360 / 256.0f : e->xRot;
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
    // 4J - The original code did not add the 1/64.0f like the teleport above
    // did, which caused minecarts to fall through the ground
    double y = e->yp / 32.0 + 1 / 64.0f;
    double z = e->zp / 32.0;
    // 4J - have changed rotation to be relative here too
    e->yRotp += packet->yRot;
    e->xRotp += packet->xRot;
    float yRot = (e->yRotp * 360) / 256.0f;
    float xRot = (e->xRotp * 360) / 256.0f;
    //    float yRot = packet->hasRot ? packet->yRot * 360 / 256.0f : e->yRot;
    //    float xRot = packet->hasRot ? packet->xRot * 360 / 256.0f : e->xRot;
    e->lerpTo(x, y, z, yRot, xRot, 3);
}

void ClientConnection::handleRemoveEntity(
    std::shared_ptr<RemoveEntitiesPacket> packet) {
    for (int i = 0; i < packet->ids.length; i++) {
        level->removeEntity(packet->ids[i]);
    }
}

void ClientConnection::handleMovePlayer(
    std::shared_ptr<MovePlayerPacket> packet) {
    std::shared_ptr<Player> player =
        minecraft->localplayers[m_userIndex];  // minecraft->player;

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
        if (!g_NetworkManager.IsHost()) {
            Minecraft::GetInstance()->progressRenderer->progressStagePercentage(
                (eCCConnected * 100) / (eCCConnected));
        }
        player->xo = player->x;
        player->yo = player->y;
        player->zo = player->z;
        // 4J - added setting xOld/yOld/zOld here too, as otherwise at the start
        // of the game we interpolate the player position from the origin to
        // wherever its first position really is
        player->xOld = player->x;
        player->yOld = player->y;
        player->zOld = player->z;

        started = true;
        minecraft->setScreen(nullptr);

        // Fix for #105852 - TU12: Content: Gameplay: Local splitscreen Players
        // are spawned at incorrect places after re-joining previously saved and
        // loaded "Mass Effect World". Move this check from
        // Minecraft::createExtraLocalPlayer 4J-PB - can't call this when this
        // function is called from the qnet thread (GetGameStarted will be
        // false)
        if (app.GetGameStarted()) {
            ui.CloseUIScenes(m_userIndex);
        }
    }
}

// 4J Added
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
    // 4J - changed to encode level in packet
    MultiPlayerLevel* dimensionLevel =
        (MultiPlayerLevel*)minecraft->levels[packet->levelIdx];
    if (dimensionLevel) {
        PIXBeginNamedEvent(0, "Handle chunk tiles update");
        LevelChunk* lc = dimensionLevel->getChunk(packet->xc, packet->zc);
        int xo = packet->xc * 16;
        int zo = packet->zc * 16;
        // 4J Stu - Unshare before we make any changes incase the server is
        // already another step ahead of us Fix for #7904 - Gameplay: Players
        // can dupe torches by throwing them repeatedly into water. This is
        // quite expensive to do, so only consider unsharing if this tile
        // setting is going to actually change something
        bool forcedUnshare = false;
        for (int i = 0; i < packet->count; i++) {
            int pos = packet->positions[i];
            int tile = packet->blocks[i] & 0xff;
            int data = packet->data[i];

            int x = (pos >> 12) & 15;
            int z = (pos >> 8) & 15;
            int y = ((pos) & 255);

            // If this is going to actually change a tile, we'll need to unshare
            int prevTile = lc->getTile(x, y, z);
            if ((tile != prevTile && !forcedUnshare)) {
                PIXBeginNamedEvent(0, "Chunk data unsharing\n");
                dimensionLevel->unshareChunkAt(xo, zo);
                PIXEndNamedEvent();
                forcedUnshare = true;
            }

            // 4J - Changes now that lighting is done at the client side of
            // things... Note - the java version now calls the doSetTileAndData
            // method from the level here rather than the levelchunk, which
            // ultimately ends up calling checkLight for the altered tile. For
            // us this doesn't always work as when sharing tile data between a
            // local server & client, the tile might not be considered to be
            // being changed on the client as the server already has changed the
            // shared data, and so the checkLight doesn't happen. Hence doing an
            // explicit checkLight here instead.
            lc->setTileAndData(x, y, z, tile, data);
            dimensionLevel->checkLight(x + xo, y, z + zo);

            dimensionLevel->clearResetRegion(x + xo, y, z + zo, x + xo, y,
                                             z + zo);

            // Don't bother setting this to dirty if it isn't going to visually
            // change - we get a lot of water changing from static to dynamic
            // for instance
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

            // 4J - remove any tite entities in this region which are associated
            // with a tile that is now no longer a tile entity. Without doing
            // this we end up with stray tile entities kicking round, which
            // leads to a bug where chests can't be properly placed again in a
            // location after (say) a chest being removed by TNT
            dimensionLevel->removeUnusedTileEntitiesInRegion(
                xo + x, y, zo + z, xo + x + 1, y + 1, zo + z + 1);
        }
        PIXBeginNamedEvent(0, "Chunk data sharing\n");
        dimensionLevel->shareChunkAt(xo,
                                     zo);  // 4J - added - only shares if chunks
                                           // are same on server & client
        PIXEndNamedEvent();

        PIXEndNamedEvent();
    }
}

void ClientConnection::handleBlockRegionUpdate(
    std::shared_ptr<BlockRegionUpdatePacket> packet) {
    // 4J - changed to encode level in packet
    MultiPlayerLevel* dimensionLevel =
        (MultiPlayerLevel*)minecraft->levels[packet->levelIdx];
    if (dimensionLevel) {
        PIXBeginNamedEvent(0, "Handle block region update");

        int y1 = packet->y + packet->ys;
        if (packet->bIsFullChunk) {
            y1 = Level::maxBuildHeight;
            if (packet->buffer.length > 0) {
                PIXBeginNamedEvent(0, "Reordering to XZY");
                LevelChunk::reorderBlocksAndDataToXZY(packet->y, packet->xs,
                                                      packet->ys, packet->zs,
                                                      &packet->buffer);
                PIXEndNamedEvent();
            }
        }
        PIXBeginNamedEvent(0, "Clear rest region");
        dimensionLevel->clearResetRegion(packet->x, packet->y, packet->z,
                                         packet->x + packet->xs - 1, y1 - 1,
                                         packet->z + packet->zs - 1);
        PIXEndNamedEvent();

        PIXBeginNamedEvent(0, "setBlocksAndData");
        // Only full chunks send lighting information now - added flag to end of
        // this call
        dimensionLevel->setBlocksAndData(packet->x, packet->y, packet->z,
                                         packet->xs, packet->ys, packet->zs,
                                         packet->buffer, packet->bIsFullChunk);
        PIXEndNamedEvent();

        //		OutputDebugString("END BRU\n");

        PIXBeginNamedEvent(0, "removeUnusedTileEntitiesInRegion");
        // 4J - remove any tite entities in this region which are associated
        // with a tile that is now no longer a tile entity. Without doing this
        // we end up with stray tile entities kicking round, which leads to a
        // bug where chests can't be properly placed again in a location after
        // (say) a chest being removed by TNT
        dimensionLevel->removeUnusedTileEntitiesInRegion(
            packet->x, packet->y, packet->z, packet->x + packet->xs, y1,
            packet->z + packet->zs);
        PIXEndNamedEvent();

        // If this is a full packet for a chunk, make sure that the cache now
        // considers that it has data for this chunk - this is used to determine
        // whether to bother rendering mobs or not, so we don't have them in
        // crazy positions before the data is there
        if (packet->bIsFullChunk) {
            PIXBeginNamedEvent(0, "dateReceivedForChunk");
            dimensionLevel->dataReceivedForChunk(packet->x >> 4,
                                                 packet->z >> 4);
            PIXEndNamedEvent();
        }
        PIXEndNamedEvent();
    }
}

void ClientConnection::handleTileUpdate(
    std::shared_ptr<TileUpdatePacket> packet) {
    // 4J added - using a block of 255 to signify that this is a packet for
    // destroying a tile, where we need to inform the level renderer that we are
    // about to do so. This is used in creative mode as the point where a tile
    // is first destroyed at the client end of things. Packets formed like this
    // are potentially sent from ServerPlayerGameMode::destroyBlock
    bool destroyTilePacket = false;
    if (packet->block == 255) {
        packet->block = 0;
        destroyTilePacket = true;
    }
    // 4J - changed to encode level in packet
    MultiPlayerLevel* dimensionLevel =
        (MultiPlayerLevel*)minecraft->levels[packet->levelIdx];
    if (dimensionLevel) {
        PIXBeginNamedEvent(0, "Handle tile update");

        if (g_NetworkManager.IsHost()) {
            // 4J Stu - Unshare before we make any changes incase the server is
            // already another step ahead of us Fix for #7904 - Gameplay:
            // Players can dupe torches by throwing them repeatedly into water.
            // This is quite expensive to do, so only consider unsharing if this
            // tile setting is going to actually change something
            int prevTile =
                dimensionLevel->getTile(packet->x, packet->y, packet->z);
            int prevData =
                dimensionLevel->getData(packet->x, packet->y, packet->z);
            if (packet->block != prevTile || packet->data != prevData) {
                PIXBeginNamedEvent(0, "Chunk data unsharing\n");
                dimensionLevel->unshareChunkAt(packet->x, packet->z);
                PIXEndNamedEvent();
            }
        }

        // 4J - In creative mode, we don't update the tile locally then get it
        // confirmed by the server - the first point that we know we are about
        // to destroy a tile is here. Let the rendering side of thing know so we
        // can synchronise collision with async render data upates.
        if (destroyTilePacket) {
            minecraft->levelRenderer->destroyedTileManager->destroyingTileAt(
                dimensionLevel, packet->x, packet->y, packet->z);
        }

        PIXBeginNamedEvent(0, "Setting data\n");
        bool tileWasSet = dimensionLevel->doSetTileAndData(
            packet->x, packet->y, packet->z, packet->block, packet->data);

        PIXEndNamedEvent();

        // 4J - remove any tite entities in this region which are associated
        // with a tile that is now no longer a tile entity. Without doing this
        // we end up with stray tile entities kicking round, which leads to a
        // bug where chests can't be properly placed again in a location after
        // (say) a chest being removed by TNT
        dimensionLevel->removeUnusedTileEntitiesInRegion(
            packet->x, packet->y, packet->z, packet->x + 1, packet->y + 1,
            packet->z + 1);

        PIXBeginNamedEvent(0, "Sharing data\n");
        dimensionLevel->shareChunkAt(
            packet->x, packet->z);  // 4J - added - only shares if chunks are
                                    // same on server & client
        PIXEndNamedEvent();

        PIXEndNamedEvent();
    }
}

void ClientConnection::handleDisconnect(
    std::shared_ptr<DisconnectPacket> packet) {
#if defined(__linux__)
    // Linux fix: On local host connections, ignore DisconnectPacket. The
    // singleplayer internal server should never disconnect itself. If we see
    // this, it's likely stream desync reading garbage data as a
    // DisconnectPacket.
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

    app.SetAction(m_userIndex, eAppAction_ExitWorld, (void*)true);
    // minecraft->setLevel(nullptr);
    // minecraft->setScreen(new DisconnectedScreen(L"disconnect.disconnected",
    // L"disconnect.genericReason", &packet->reason));
}

void ClientConnection::onDisconnect(DisconnectPacket::eDisconnectReason reason,
                                    void* reasonObjects) {
    if (done) return;
    done = true;

    Minecraft* pMinecraft = Minecraft::GetInstance();
    pMinecraft->connectionDisconnected(m_userIndex, reason);

    // 4J Stu - TU-1 hotfix
    // Fix for #13191 - The host of a game can get a message informing them that
    // the connection to the server has been lost In the (now unlikely) event
    // that the host connections times out, allow the player to save their game
    if (g_NetworkManager.IsHost() &&
        (reason == DisconnectPacket::eDisconnect_TimeOut ||
         reason == DisconnectPacket::eDisconnect_Overflow) &&
        m_userIndex == ProfileManager.GetPrimaryPad() &&
        !MinecraftServer::saveOnExitAnswered()) {
        unsigned int uiIDA[1];
        uiIDA[0] = IDS_CONFIRM_OK;
        ui.RequestErrorMessage(IDS_EXITING_GAME, IDS_GENERIC_ERROR, uiIDA, 1,
                               ProfileManager.GetPrimaryPad(),
                               &ClientConnection::HostDisconnectReturned,
                               nullptr);
    } else {
        app.SetAction(m_userIndex, eAppAction_ExitWorld, (void*)true);
    }

    // minecraft->setLevel(nullptr);
    // minecraft->setScreen(new DisconnectedScreen(L"disconnect.lost", reason,
    // reasonObjects));
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

    // 4J - the original game could assume that if getEntity didn't find the
    // player, it must be the local player. We need to search all local players
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
        // Don't know if this should ever really happen, but seems safest to try
        // and remove the entity that has been collected even if we can't create
        // a particle as we don't know what really collected it
        level->removeEntity(packet->itemId);
        return;
    }

    if (from != nullptr) {
        // If this is a local player, then we only want to do processing for it
        // if this connection is associated with the player it is for. In
        // particular, we don't want to remove the item entity until we are
        // processing it for the right connection, or else we won't have a valid
        // "from" reference if we've already removed the item for an earlier
        // processed connection
        if (isLocalPlayer) {
            std::shared_ptr<LocalPlayer> player =
                std::dynamic_pointer_cast<LocalPlayer>(to);

            // 4J Stu - Fix for #10213 - UI: Local clients cannot progress
            // through the tutorial normally. We only send this packet once if
            // many local players can see the event, so make sure we update the
            // tutorial for the player that actually picked up the item
            int playerPad = player->GetXboxPad();

            if (minecraft->localgameModes[playerPad] != nullptr) {
                // 4J-PB - add in the XP orb sound
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
                // Don't know if this should ever really happen, but seems
                // safest to try and remove the entity that has been collected
                // even if it somehow isn't an itementity
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

    // On platforms other than Xbox One this just sets display name to gamertag
    if (packet->m_stringArgs.size() >= 1)
        playerDisplayName = GetDisplayNameByGamertag(packet->m_stringArgs[0]);
    if (packet->m_stringArgs.size() >= 2)
        sourceDisplayName = GetDisplayNameByGamertag(packet->m_stringArgs[1]);

    switch (packet->m_messageType) {
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
            // message=app.GetString(IDS_DEATH_FELL_KILLER);
            // replacePlayer = true;
            // replaceEntitySource = true;

            // 4J Stu - The correct string for here, IDS_DEATH_FELL_KILLER is
            // incorrect. We can't change localisation, so use a different
            // string for now
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
            // Spawn eggs
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

            // Breeding
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

            // can't shear the mooshroom
        case ChatPacket::e_ChatPlayerCantShearMooshroom:
            message = app.GetString(IDS_CANT_SHEAR_MOOSHROOM);
            break;

            // Paintings/Item Frames
        case ChatPacket::e_ChatPlayerMaxHangingEntities:
            message = app.GetString(IDS_MAX_HANGINGENTITIES);
            break;
            // Enemy spawn eggs in peaceful
        case ChatPacket::e_ChatPlayerCantSpawnInPeaceful:
            message = app.GetString(IDS_CANT_SPAWN_IN_PEACEFUL);
            break;

            // Enemy spawn eggs in peaceful
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

            // Check for a custom mob name
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

    // flag that a message is a death message
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

        if (player == minecraft->localplayers[m_userIndex]) {
            TelemetryManager->RecordEnemyKilledOrOvercome(
                m_userIndex, 0, player->y, 0, 0, 0, 0, eTelemetryInGame_UseBed);
        }
    }
}

void ClientConnection::handlePreLogin(std::shared_ptr<PreLoginPacket> packet) {
    fprintf(stderr,
            "[LOGIN-CLI] handlePreLogin entered, isHost=%d, userIdx=%d\n",
            (int)g_NetworkManager.IsHost(), m_userIndex);
    // 4J - Check that we can play with all the players already in the game who
    // have Friends-Only UGC set
    bool canPlay = true;
    bool canPlayLocal = true;
    bool isAtLeastOneFriend = g_NetworkManager.IsHost();
    bool isFriendsWithHost = true;
    bool cantPlayContentRestricted = false;

    if (!g_NetworkManager.IsHost()) {
        // set the game host settings
        app.SetGameHostOption(eGameHostOption_All, packet->m_serverSettings);

        // 4J-PB - if we go straight in from the menus via an invite, we won't
        // have the DLC info
        if (app.GetTMSGlobalFileListRead() == false) {
            app.SetTMSAction(ProfileManager.GetPrimaryPad(),
                             eTMSAction_TMSPP_RetrieveFiles_RunPlayGame);
        }
    }

    // TODO - handle this kind of things for non-360 platforms
    canPlay = true;
    canPlayLocal = true;
    isAtLeastOneFriend = true;
    cantPlayContentRestricted = false;

    if (!canPlay || !canPlayLocal || !isAtLeastOneFriend ||
        cantPlayContentRestricted) {
        DisconnectPacket::eDisconnectReason reason =
            DisconnectPacket::eDisconnect_NoUGC_Remote;
        if (m_userIndex == ProfileManager.GetPrimaryPad()) {
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
            app.SetAction(ProfileManager.GetPrimaryPad(), eAppAction_ExitWorld,
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

            // 4J-PB - this locks up on the read and write threads not closing
            // down, because they are trying to lock the incoming critsec when
            // it's already locked by this thread
            // 			Minecraft::GetInstance()->connectionDisconnected(
            // m_userIndex , reason ); 			done = true;
            // connection->flush(); connection->close(reason);
            //			app.SetAction(m_userIndex,eAppAction_ExitPlayer);

            // 4J-PB - doing this instead
            app.SetAction(m_userIndex, eAppAction_ExitPlayerPreLogin);
        }
    } else {
        // Texture pack handling
        // If we have the texture pack for the game, load it
        // If we don't then send a packet to the host to request it. We need to
        // send this before the LoginPacket so that it gets handled first, as
        // once the LoginPacket is received on the client the game is close to
        // starting
        if (m_userIndex == ProfileManager.GetPrimaryPad()) {
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

                // 4J-PB - we need to upsell the texture pack to the player
                // Let the player go into the game, and we'll check that they
                // are using the right texture pack when in
            }
        }

        if (!g_NetworkManager.IsHost()) {
            Minecraft::GetInstance()->progressRenderer->progressStagePercentage(
                (eCCPreLoginReceived * 100) / (eCCConnected));
        }
        // need to use the XUID here
        PlayerUID offlineXUID = INVALID_XUID;
        PlayerUID onlineXUID = INVALID_XUID;
        if (ProfileManager.IsSignedInLive(m_userIndex)) {
            // Guest don't have an offline XUID as they cannot play offline, so
            // use their online one
            ProfileManager.GetXUID(m_userIndex, &onlineXUID, true);
        }

        // On PS3, all non-signed in players (even guests) can get a useful
        // offlineXUID
        if (!ProfileManager.IsGuest(m_userIndex)) {
            // All other players we use their offline XUID so that they can play
            // the game offline
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
        send(std::shared_ptr<LoginPacket>(new LoginPacket(
            minecraft->user->name, SharedConstants::NETWORK_PROTOCOL_VERSION,
            offlineXUID, onlineXUID, (!allAllowed && friendsAllowed),
            packet->m_ugcPlayersVersion, app.GetPlayerSkinId(m_userIndex),
            app.GetPlayerCapeId(m_userIndex),
            ProfileManager.IsGuest(m_userIndex))));
        fprintf(stderr, "[LOGIN] LoginPacket sent successfully\n");

        if (!g_NetworkManager.IsHost()) {
            Minecraft::GetInstance()->progressRenderer->progressStagePercentage(
                (eCCLoginSent * 100) / (eCCConnected));
        }
    }
}

void ClientConnection::close() {
    // If it's already done, then we don't need to do anything here. And in fact
    // trying to do something could cause a crash
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
    mob->xp = packet->x;
    mob->yp = packet->y;
    mob->zp = packet->z;
    mob->yHeadRot = packet->yHeadRot * 360 / 256.0f;
    mob->yRotp = packet->yRot;
    mob->xRotp = packet->xRot;

    std::vector<std::shared_ptr<Entity> >* subEntities = mob->getSubEntities();
    if (subEntities != nullptr) {
        int offs = packet->id - mob->entityId;
        // for (int i = 0; i < subEntities.length; i++)
        for (auto it = subEntities->begin(); it != subEntities->end(); ++it) {
            // subEntities[i].entityId += offs;
            (*it)->entityId += offs;
        }
    }

    mob->entityId = packet->id;

    //	printf("\t\t\t\t%d: Add mob rot %d\n",packet->id,packet->yRot);

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

    // Fix for #65236 - TU8: Content: Gameplay: Magma Cubes' have strange hit
    // boxes. 4J Stu - Slimes have a different BB depending on their size which
    // is set in the entity data, so update the BB
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
    // minecraft->player->setRespawnPosition(new Pos(packet->x, packet->y,
    // packet->z));
    minecraft->localplayers[m_userIndex]->setRespawnPosition(
        new Pos(packet->x, packet->y, packet->z), true);
    minecraft->level->getLevelData()->setSpawn(packet->x, packet->y, packet->z);
}

void ClientConnection::handleEntityLinkPacket(
    std::shared_ptr<SetEntityLinkPacket> packet) {
    std::shared_ptr<Entity> sourceEntity = getEntity(packet->sourceId);
    std::shared_ptr<Entity> destEntity = getEntity(packet->destId);

    // 4J: If the destination entity couldn't be found, defer handling of this
    // packet This was added to support leashing (the entity link packet is sent
    // before the add entity packet)
    if (destEntity == nullptr && packet->destId >= 0) {
        // We don't handle missing source entities because it shouldn't happen
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

        // 4J TODO: pretty sure this message is a tooltip so not needed
        /*
        if (displayMountMessage) {
                Options options = minecraft.options;
                minecraft.gui.setOverlayMessage(I18n.get("mount.onboard",
        Options.getTranslatedKeyMessage(options.keySneak.key)), false);
        }
        */
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
    // if (entityId == minecraft->player->entityId)
    if (entityId == minecraft->localplayers[m_userIndex]->entityId) {
        // return minecraft->player;
        return minecraft->localplayers[m_userIndex];
    }
    return level->getEntity(entityId);
}

void ClientConnection::handleSetHealth(
    std::shared_ptr<SetHealthPacket> packet) {
    // minecraft->player->hurtTo(packet->health);
    minecraft->localplayers[m_userIndex]->hurtTo(packet->health,
                                                 packet->damageSource);
    minecraft->localplayers[m_userIndex]->getFoodData()->setFoodLevel(
        packet->food);
    minecraft->localplayers[m_userIndex]->getFoodData()->setSaturation(
        packet->saturation);

    // We need food
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
    // Both PlayerConnection and ClientConnection should handle this mostly the
    // same way Server side also needs to store a list of those clients waiting
    // to get a texture the server doesn't have yet so that it can send it out
    // to them when it comes in

    if (packet->dataBytes == 0) {
        // Request for texture
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
        // Response with texture data
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
    // Both PlayerConnection and ClientConnection should handle this mostly the
    // same way Server side also needs to store a list of those clients waiting
    // to get a texture the server doesn't have yet so that it can send it out
    // to them when it comes in

    if (packet->dwTextureBytes == 0) {
        // Request for texture
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
        // Response with texture data
#if !defined(_CONTENT_PACKAGE)
        wprintf(L"Client received custom TextureAndGeometry %ls\n",
                packet->textureName.c_str());
#endif
        // Add the texture data
        app.AddMemoryTextureFile(packet->textureName, packet->pbData,
                                 packet->dwTextureBytes);
        // Add the geometry data
        if (packet->dwBoxC != 0) {
            app.SetAdditionalSkinBoxes(packet->dwSkinID, packet->BoxDataA,
                                       packet->dwBoxC);
        }
        // Add the anim override
        app.SetAnimOverrideBitmask(packet->dwSkinID,
                                   packet->uiAnimOverrideBitmask);

        // clear out the pending texture request
        Minecraft::GetInstance()->handleClientTextureReceived(
            packet->textureName);
    }
}

void ClientConnection::handleTextureChange(
    std::shared_ptr<TextureChangePacket> packet) {
    std::shared_ptr<Entity> e = getEntity(packet->id);
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
            player->setCustomSkin(app.getSkinIdFromPath(packet->path));
#if !defined(_CONTENT_PACKAGE)
            wprintf(L"Skin for remote player %ls has changed to %ls (%d)\n",
                    player->name.c_str(), player->customTextureUrl.c_str(),
                    player->getPlayerDefaultSkin());
#endif
            break;
        case TextureChangePacket::e_TextureChange_Cape:
            player->setCustomCape(Player::getCapeIdFromPath(packet->path));
            // player->customTextureUrl2 = packet->path;
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
        // Update the ref count on the memory texture data
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
        // Update the ref count on the memory texture data
        app.AddMemoryTextureFile(packet->path, nullptr, 0);
    }
}

void ClientConnection::handleRespawn(std::shared_ptr<RespawnPacket> packet) {
    // if (packet->dimension != minecraft->player->dimension)
    if (packet->dimension != minecraft->localplayers[m_userIndex]->dimension ||
        packet->mapSeed !=
            minecraft->localplayers[m_userIndex]->level->getSeed()) {
        int oldDimension = minecraft->localplayers[m_userIndex]->dimension;
        started = false;

        // Remove client connection from this level
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

            // 4J Stu - We want to shared the savedDataStorage between both
            // levels
            // if( dimensionLevel->savedDataStorage != nullptr )
            //{
            // Don't need to delete it here as it belongs to a client connection
            // while will delete it when it's done
            //	delete dimensionLevel->savedDataStorage;+
            //}
            dimensionLevel->savedDataStorage = level->savedDataStorage;

            dimensionLevel->difficulty = packet->difficulty;  // 4J Added
            app.DebugPrintf("dimensionLevel->difficulty - Difficulty = %d\n",
                            packet->difficulty);

            dimensionLevel->isClientSide = true;
        } else {
            dimensionLevel->addClientConnection(this);
        }

        // Remove the player entity from the current level
        level->removeEntity(
            std::shared_ptr<Entity>(minecraft->localplayers[m_userIndex]));

        level = dimensionLevel;

        // Whilst calling setLevel, make sure that minecraft::player is set up
        // to be correct for this connection
        std::shared_ptr<MultiplayerLocalPlayer> lastPlayer = minecraft->player;
        minecraft->player = minecraft->localplayers[m_userIndex];
        minecraft->setLevel(dimensionLevel);
        minecraft->player = lastPlayer;

        TelemetryManager->RecordLevelExit(m_userIndex,
                                          eSen_LevelExitStatus_Succeeded);

        // minecraft->player->dimension = packet->dimension;
        minecraft->localplayers[m_userIndex]->dimension = packet->dimension;
        minecraft->setScreen(new ReceivingLevelScreen(this));
        //		minecraft->addPendingLocalConnection(m_userIndex, this);

        if (minecraft->localgameModes[m_userIndex] != nullptr) {
            TutorialMode* gameMode =
                (TutorialMode*)minecraft->localgameModes[m_userIndex];
            gameMode->getTutorial()->showTutorialPopup(false);
        }

        // 4J-JEV: Fix for Durango #156334 - Content: UI: Rich Presence 'In the
        // Nether' message is updating with a 3 to 10 minute  delay.
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

        // 4J Stu - Fix for #13543 - Crash: Game crashes if entering a portal
        // with the inventory menu open
        ui.CloseUIScenes(m_userIndex);

        if (app.GetLocalPlayerCount() > 1) {
            ui.NavigateToScene(m_userIndex, eUIScene_ConnectingProgress, param);
        } else {
            ui.NavigateToScene(m_userIndex, eUIScene_ConnectingProgress, param);
        }

        app.SetAction(m_userIndex, eAppAction_WaitForDimensionChangeComplete);
    }

    // minecraft->respawnPlayer(minecraft->player->GetXboxPad(),true,
    // packet->dimension);

    // Wrap respawnPlayer call up in code to set & restore the player/gamemode
    // etc. as some things in there assume that we are set up for the player
    // that the respawn is coming in for
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
        // app.DebugPrintf("Received ExplodePacket with explosion data\n");
        PIXBeginNamedEvent(0, "Handling explosion");
        Explosion* e = new Explosion(minecraft->level, nullptr, packet->x,
                                     packet->y, packet->z, packet->r);
        PIXBeginNamedEvent(0, "Finalizing");

        // Fix for #81758 - TCR 006 BAS Non-Interactive Pause: TU9: Performance:
        // Gameplay: After detonating bunch of TNT, game enters unresponsive
        // state for couple of seconds. The changes we are making here have been
        // decided by the server, so we don't need to add them to the vector
        // that resets tiles changes made on the client as we KNOW that the
        // server is matching these changes
        MultiPlayerLevel* mpLevel = (MultiPlayerLevel*)minecraft->level;
        mpLevel->enableResetChanges(false);
        // 4J - now directly pass a pointer to the toBlow array in the packet
        // rather than copying around
        e->finalizeExplosion(true, &packet->toBlow);
        mpLevel->enableResetChanges(true);
        PIXEndNamedEvent();
        PIXEndNamedEvent();
        delete e;
    } else {
        // app.DebugPrintf("Received ExplodePacket with knockback only data\n");
    }

    // app.DebugPrintf("Adding knockback (%f,%f,%f) for player %d\n",
    // packet->getKnockbackX(), packet->getKnockbackY(),
    // packet->getKnockbackZ(), m_userIndex);
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
                std::shared_ptr<HopperTileEntity>(new HopperTileEntity());
            if (packet->customName) hopper->setCustomName(packet->title);
            if (player->openHopper(hopper)) {
                player->containerMenu->containerId = packet->containerId;
            } else {
                failed = true;
            }
        } break;
        case ContainerOpenPacket::FURNACE: {
            std::shared_ptr<FurnaceTileEntity> furnace =
                std::shared_ptr<FurnaceTileEntity>(new FurnaceTileEntity());
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
                std::shared_ptr<DropperTileEntity>(new DropperTileEntity());
            if (packet->customName) dropper->setCustomName(packet->title);

            if (player->openTrap(dropper)) {
                player->containerMenu->containerId = packet->containerId;
            } else {
                failed = true;
            }
        } break;
        case ContainerOpenPacket::TRAP: {
            std::shared_ptr<DispenserTileEntity> dispenser =
                std::shared_ptr<DispenserTileEntity>(new DispenserTileEntity());
            if (packet->customName) dispenser->setCustomName(packet->title);

            if (player->openTrap(dispenser)) {
                player->containerMenu->containerId = packet->containerId;
            } else {
                failed = true;
            }
        } break;
        case ContainerOpenPacket::WORKBENCH: {
            if (player->startCrafting(Mth::floor(player->x),
                                      Mth::floor(player->y),
                                      Mth::floor(player->z))) {
                player->containerMenu->containerId = packet->containerId;
            } else {
                failed = true;
            }
        } break;
        case ContainerOpenPacket::ENCHANTMENT: {
            if (player->startEnchanting(
                    Mth::floor(player->x), Mth::floor(player->y),
                    Mth::floor(player->z),
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
                std::shared_ptr<BeaconTileEntity>(new BeaconTileEntity());
            if (packet->customName) beacon->setCustomName(packet->title);

            if (player->openBeacon(beacon)) {
                player->containerMenu->containerId = packet->containerId;
            } else {
                failed = true;
            }
        } break;
        case ContainerOpenPacket::REPAIR_TABLE: {
            if (player->startRepairing(Mth::floor(player->x),
                                       Mth::floor(player->y),
                                       Mth::floor(player->z))) {
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
            if (player->openFireworks(Mth::floor(player->x),
                                      Mth::floor(player->y),
                                      Mth::floor(player->z))) {
                player->containerMenu->containerId = packet->containerId;
            } else {
                failed = true;
            }
        } break;
    }

    if (failed) {
        // Failed - if we've got a non-inventory container currently here, close
        // that, which locally should put us back to not having a container
        // open, and should send a containerclose to the server so it doesn't
        // have a container open. If we don't have a non-inventory container
        // open, just send the packet, and again we ought to be in sync with the
        // server.
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
            // 4J Stu - Reworked a bit to fix a bug where things being collected
            // while the creative menu was up replaced items in the creative
            // menu
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
            send(std::shared_ptr<ContainerAckPacket>(new ContainerAckPacket(
                packet->containerId, packet->uid, true)));
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
            std::shared_ptr<SignTileEntity>(new SignTileEntity());
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

        // 4J-PB - on a client connecting, the line below fails
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
        // 4J Stu - Brought forward change from 1.3 to fix #64688 - Customer
        // Encountered: TU7: Content: Art: Aura of enchanted item is not
        // displayed for other players in online game
        entity->setEquippedSlot(packet->slot, packet->getItem());
    }
}

void ClientConnection::handleContainerClose(
    std::shared_ptr<ContainerClosePacket> packet) {
    minecraft->localplayers[m_userIndex]->clientSideCloseContainer();
}

void ClientConnection::handleTileEvent(
    std::shared_ptr<TileEventPacket> packet) {
    PIXBeginNamedEvent(0, "Handle tile event\n");
    minecraft->level->tileEvent(packet->x, packet->y, packet->z, packet->tile,
                                packet->b0, packet->b1);
    PIXEndNamedEvent();
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
            0)  // 4J - was nullptr check
        {
            minecraft->localplayers[m_userIndex]->displayClientMessage(
                GameEventPacket::EVENT_LANGUAGE_ID[event]);
        }
    }
    if (event == GameEventPacket::START_RAINING) {
        level->getLevelData()->setRaining(true);
        level->setRainLevel(1);
    } else if (event == GameEventPacket::STOP_RAINING) {
        level->getLevelData()->setRaining(false);
        level->setRainLevel(0);
    } else if (event == GameEventPacket::CHANGE_GAME_MODE) {
        minecraft->localgameModes[m_userIndex]->setLocalMode(
            GameType::byId(param));
    } else if (event == GameEventPacket::WIN_GAME) {
        ui.SetWinUserIndex(static_cast<unsigned int>(gameEventPacket->param));

        app.DebugPrintf("handleGameEvent packet for WIN_GAME - %d\n",
                        m_userIndex);
        // This just allows it to be shown
        if (minecraft->localgameModes[ProfileManager.GetPrimaryPad()] !=
            nullptr)
            minecraft->localgameModes[ProfileManager.GetPrimaryPad()]
                ->getTutorial()
                ->showTutorialPopup(false);
        ui.NavigateToScene(ProfileManager.GetPrimaryPad(), eUIScene_EndPoem,
                           nullptr, eUILayer_Scene, eUIGroup_Fullscreen);
    } else if (event == GameEventPacket::START_SAVING) {
        if (!g_NetworkManager.IsHost()) {
            // Move app started to here so that it happens immediately otherwise
            // back-to-back START/STOP packets leave the client stuck in the
            // loading screen
            app.SetGameStarted(false);
            app.SetAction(ProfileManager.GetPrimaryPad(),
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
        //		System.out.println("Unknown itemid: " + packet->itemId);
        //// 4J removed
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
    minecraft->localplayers[m_userIndex]->awardStatFromServer(
        GenericStats::stat(packet->statId), packet->getParamData());
}

void ClientConnection::handleUpdateMobEffect(
    std::shared_ptr<UpdateMobEffectPacket> packet) {
    std::shared_ptr<Entity> e = getEntity(packet->entityId);
    if ((e == nullptr) || !e->instanceof(eTYPE_LIVINGENTITY)) return;

    //( std::dynamic_pointer_cast<LivingEntity>(e) )->addEffect(new
    // MobEffectInstance(packet->effectId, packet->effectDurationTicks,
    // packet->effectAmplifier));

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
        // Some settings should always be considered on for the host player
        Player::enableAllPlayerPrivileges(startingPrivileges, true);
        Player::setPlayerGamePrivilege(startingPrivileges,
                                       Player::ePlayerGamePrivilege_HOST, 1);
    }

    // 4J Stu - Repurposed this packet for player info that we want
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

    // 4J Stu - I don't think we care about this, so not converting it (came
    // from 1.8.2)
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
    send(std::shared_ptr<KeepAlivePacket>(new KeepAlivePacket(packet->id)));
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
        // 4jcraft: use the java gui getMerchant() to get trader as we don't
        // have iggy's screen anymore
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

// 4J Added
void ClientConnection::handleServerSettingsChanged(
    std::shared_ptr<ServerSettingsChangedPacket> packet) {
    if (packet->action == ServerSettingsChangedPacket::HOST_IN_GAME_SETTINGS) {
        app.SetGameHostOption(eGameHostOption_All, packet->data);
    } else if (packet->action == ServerSettingsChangedPacket::HOST_DIFFICULTY) {
        for (unsigned int i = 0; i < minecraft->levels.length; ++i) {
            if (minecraft->levels[i] != nullptr) {
                app.DebugPrintf(
                    "ClientConnection::handleServerSettingsChanged - "
                    "Difficulty = %d",
                    packet->data);
                minecraft->levels[i]->difficulty = packet->data;
            }
        }
    } else {
        // options
        // minecraft->options->SetGamertagSetting((packet->data==0)?false:true);
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
            packet->m_definitionType, message, packet->m_data.data,
            packet->m_data.length);
        if (minecraft->localgameModes[m_userIndex] != nullptr) {
            minecraft->localgameModes[m_userIndex]->getTutorial()->setMessage(
                message, packet->m_icon, packet->m_auxValue);
        }
    }
    // If this rule has a data tag associated with it, then we save that in user
    // profile data
    if (packet->m_dataTag > 0 && packet->m_dataTag <= 32) {
        app.DebugPrintf(
            "handleUpdateGameRuleProgressPacket: Data tag is in range, so "
            "updating profile data\n");
        app.SetSpecialTutorialCompletionFlag(m_userIndex,
                                             packet->m_dataTag - 1);
    }
    delete[] packet->m_data.data;
}

// 4J Stu - TU-1 hotfix
// Fix for #13191 - The host of a game can get a message informing them that the
// connection to the server has been lost
int ClientConnection::HostDisconnectReturned(
    void* pParam, int iPad, C4JStorage::EMessageResult result) {
    // 4J-PB - if they have a trial texture pack, they don't get to save the
    // world
    if (!Minecraft::GetInstance()->skins->isUsingDefaultSkin()) {
        TexturePack* tPack = Minecraft::GetInstance()->skins->getSelected();
        DLCTexturePack* pDLCTexPack = (DLCTexturePack*)tPack;

        DLCPack* pDLCPack =
            pDLCTexPack->getDLCInfoParentPack();  // tPack->getDLCPack();
        if (!pDLCPack->hasPurchasedFile(DLCManager::e_DLCType_Texture, L"")) {
            // no upsell, we're about to quit
            MinecraftServer::getInstance()->setSaveOnExit(false);
            // flag a app action of exit game
            app.SetAction(iPad, eAppAction_ExitWorld);
        }
    }

    // Give the player the option to save their game
    // does the save exist?
    bool bSaveExists;
    StorageManager.DoesSaveExist(&bSaveExists);
    // 4J-PB - we check if the save exists inside the libs
    // we need to ask if they are sure they want to overwrite the existing game
    if (bSaveExists) {
        unsigned int uiIDA[2];
        uiIDA[0] = IDS_CONFIRM_CANCEL;
        uiIDA[1] = IDS_CONFIRM_OK;
        ui.RequestErrorMessage(IDS_TITLE_SAVE_GAME, IDS_CONFIRM_SAVE_GAME,
                               uiIDA, 2, ProfileManager.GetPrimaryPad(),
                               &ClientConnection::ExitGameAndSaveReturned,
                               nullptr);
    } else {
        MinecraftServer::getInstance()->setSaveOnExit(true);
        // flag a app action of exit game
        app.SetAction(iPad, eAppAction_ExitWorld);
    }

    return 0;
}

int ClientConnection::ExitGameAndSaveReturned(
    void* pParam, int iPad, C4JStorage::EMessageResult result) {
    // results switched for this dialog
    if (result == C4JStorage::EMessage_ResultDecline) {
        // int32_t saveOrCheckpointId = 0;
        // bool validSave =
        // StorageManager.GetSaveUniqueNumber(&saveOrCheckpointId);
        // SentientManager.RecordLevelSaveOrCheckpoint(ProfileManager.GetPrimaryPad(),
        // saveOrCheckpointId);
        MinecraftServer::getInstance()->setSaveOnExit(true);
    } else {
        MinecraftServer::getInstance()->setSaveOnExit(false);
    }
    // flag a app action of exit game
    app.SetAction(iPad, eAppAction_ExitWorld);
    return 0;
}

//
std::wstring ClientConnection::GetDisplayNameByGamertag(std::wstring gamertag) {
    return gamertag;
}

void ClientConnection::handleAddObjective(
    std::shared_ptr<SetObjectivePacket> packet) {}

void ClientConnection::handleSetScore(std::shared_ptr<SetScorePacket> packet) {}

void ClientConnection::handleSetDisplayObjective(
    std::shared_ptr<SetDisplayObjectivePacket> packet) {}

void ClientConnection::handleSetPlayerTeamPacket(
    std::shared_ptr<SetPlayerTeamPacket> packet) {}

void ClientConnection::handleParticleEvent(
    std::shared_ptr<LevelParticlesPacket> packet) {
    for (int i = 0; i < packet->getCount(); i++) {
        double xVarience = random->nextGaussian() * packet->getXDist();
        double yVarience = random->nextGaussian() * packet->getYDist();
        double zVarience = random->nextGaussian() * packet->getZDist();
        double xa = random->nextGaussian() * packet->getMaxSpeed();
        double ya = random->nextGaussian() * packet->getMaxSpeed();
        double za = random->nextGaussian() * packet->getMaxSpeed();

        // TODO: determine particle ID from name
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
        // Entity is not a living entity!
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
            // 4J - TODO: revisit, not familiar with the attribute system, why
            // are we passing in MIN_NORMAL (Java's smallest non-zero value
            // conforming to IEEE Standard 754 (?)) and MAX_VALUE
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

// 4J: Check for deferred entity link packets related to this entity ID and
// handle them
void ClientConnection::checkDeferredEntityLinkPackets(int newEntityId) {
    if (deferredEntityLinkPackets.empty()) return;

    for (int i = 0; i < deferredEntityLinkPackets.size(); i++) {
        DeferredEntityLinkPacket* deferred = &deferredEntityLinkPackets[i];

        bool remove = false;

        // Only consider recently deferred packets
        int tickInterval = GetTickCount() - deferred->m_recievedTick;
        if (tickInterval < MAX_ENTITY_LINK_DEFERRAL_INTERVAL) {
            // Note: we assume it's the destination entity
            if (deferred->m_packet->destId == newEntityId) {
                handleEntityLinkPacket(deferred->m_packet);
                remove = true;
            }
        } else {
            // This is an old packet, remove (shouldn't really come up but seems
            // prudent)
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
    m_recievedTick = GetTickCount();
    m_packet = packet;
}
