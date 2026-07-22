#include "PlayerChunkMap.h"

#include <assert.h>

#include <algorithm>
#include <compare>
#include <format>
#include <list>
#include <unordered_set>
#include <utility>

#include "app/common/src/Network/GameNetworkManager.h"
#include "app/common/src/Network/NetworkPlayerInterface.h"
#include "app/mac/MacGame.h"
#include "ServerChunkCache.h"
#include "ServerLevel.h"
#include "ServerPlayer.h"
#include "minecraft/network/packet/BlockRegionUpdatePacket.h"
#include "minecraft/network/packet/ChunkTilesUpdatePacket.h"
#include "minecraft/network/packet/ChunkVisibilityAreaPacket.h"
#include "minecraft/network/packet/ChunkVisibilityPacket.h"
#include "minecraft/network/packet/Packet.h"
#include "minecraft/network/packet/TileUpdatePacket.h"
#include "minecraft/server/MinecraftServer.h"
#include "minecraft/server/PlayerList.h"
#include "minecraft/server/network/PlayerConnection.h"
#include "minecraft/world/level/ChunkPos.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/chunk/LevelChunk.h"
#include "minecraft/world/level/tile/entity/TileEntity.h"

PlayerChunkMap::PlayerChunk::PlayerChunk(int x, int z, PlayerChunkMap* pcm)
    : pos(x, z) {
    
    changes = 0;
    changedTiles = std::vector<short>(MAX_CHANGES_BEFORE_RESEND);
    xChangeMin = xChangeMax = 0;
    yChangeMin = yChangeMax = 0;
    zChangeMin = zChangeMax = 0;
    parent = pcm;                 
    ticksToNextRegionUpdate = 0;  
    prioritised = false;          
    firstInhabitedTime = 0;

    parent->getLevel()->cache->create(x, z);
}

PlayerChunkMap::PlayerChunk::~PlayerChunk() {}








void PlayerChunkMap::flagEntitiesToBeRemoved(unsigned int* flags,
                                             bool* flagToBeRemoved) {
    for (auto it = players.begin(); it != players.end(); it++) {
        std::shared_ptr<ServerPlayer> serverPlayer = *it;
        serverPlayer->flagEntitiesToBeRemoved(flags, flagToBeRemoved);
    }
}

void PlayerChunkMap::PlayerChunk::add(std::shared_ptr<ServerPlayer> player,
                                      bool sendPacket ) {
    
    if (find(players.begin(), players.end(), player) != players.end()) {
        
        
        
        app.DebugPrintf(
            "--- Adding player to chunk x=%d\t z=%d, but they are already in "
            "there!\n",
            pos.x, pos.z);
        return;

        
        
        
    }

    player->seenChunks.insert(pos);

    
    if (sendPacket)
        player->connection->send(std::shared_ptr<ChunkVisibilityPacket>(
            new ChunkVisibilityPacket(pos.x, pos.z, true)));

    if (players.empty()) {
        firstInhabitedTime = parent->level->getGameTime();
    }

    players.push_back(player);

    player->chunksToSend.push_back(pos);

#ifdef _LARGE_WORLDS
    parent->getLevel()->cache->dontDrop(pos.x, pos.z);  
#endif
}

void PlayerChunkMap::PlayerChunk::remove(std::shared_ptr<ServerPlayer> player) {
    PlayerChunkMap::PlayerChunk* toDelete = nullptr;

    
    
    auto it = find(players.begin(), players.end(), player);
    if (it == players.end()) {
        app.DebugPrintf(
            "--- INFO - Removing player from chunk x=%d\t z=%d, but they are "
            "not in that chunk!\n",
            pos.x, pos.z);

        return;
    }

    players.erase(it);
    if (players.size() == 0) {
        {
            LevelChunk* chunk = parent->level->getChunk(pos.x, pos.z);
            updateInhabitedTime(chunk);
            auto it = find(parent->knownChunks.begin(),
                           parent->knownChunks.end(), this);
            if (it != parent->knownChunks.end()) parent->knownChunks.erase(it);
        }
        int64_t id = (pos.x + 0x7fffffffLL) | ((pos.z + 0x7fffffffLL) << 32);
        auto it = parent->chunks.find(id);
        if (it != parent->chunks.end()) {
            toDelete = it->second;  
                                    
            parent->chunks.erase(it);
        }
        if (changes > 0) {
            auto it = find(parent->changedChunks.begin(),
                           parent->changedChunks.end(), this);
            parent->changedChunks.erase(it);
        }
        parent->getLevel()->cache->drop(pos.x, pos.z);
    }

    player->chunksToSend.remove(pos);
    
    
    
    
    
    if (player->connection != nullptr &&
        player->seenChunks.find(pos) != player->seenChunks.end()) {
        INetworkPlayer* thisNetPlayer = player->connection->getNetworkPlayer();
        bool noOtherPlayersFound = true;

        if (thisNetPlayer != nullptr) {
            for (auto it = players.begin(); it < players.end(); ++it) {
                std::shared_ptr<ServerPlayer> currPlayer = *it;
                INetworkPlayer* currNetPlayer =
                    currPlayer->connection->getNetworkPlayer();
                if (currNetPlayer != nullptr &&
                    currNetPlayer->IsSameSystem(thisNetPlayer) &&
                    currPlayer->seenChunks.find(pos) !=
                        currPlayer->seenChunks.end()) {
                    noOtherPlayersFound = false;
                    break;
                }
            }
            if (noOtherPlayersFound) {
                
                
                player->connection->send(std::shared_ptr<ChunkVisibilityPacket>(
                    new ChunkVisibilityPacket(pos.x, pos.z, false)));

                
                
                
                
                
                
                
                
                if (!player->connection->isLocal()) {
                    int flagIndex = ServerPlayer::getFlagIndexForChunk(
                        pos, parent->dimension);
                    g_NetworkManager.SystemFlagClear(thisNetPlayer, flagIndex);
                }
            }
        } else {
            
            
        }
    }

    delete toDelete;
}

void PlayerChunkMap::PlayerChunk::updateInhabitedTime() {
    updateInhabitedTime(parent->level->getChunk(pos.x, pos.z));
}

void PlayerChunkMap::PlayerChunk::updateInhabitedTime(LevelChunk* chunk) {
    chunk->inhabitedTime += parent->level->getGameTime() - firstInhabitedTime;

    firstInhabitedTime = parent->level->getGameTime();
}

void PlayerChunkMap::PlayerChunk::tileChanged(int x, int y, int z) {
    if (changes == 0) {
        parent->changedChunks.push_back(this);
        xChangeMin = xChangeMax = x;
        yChangeMin = yChangeMax = y;
        zChangeMin = zChangeMax = z;
    }
    if (xChangeMin > x) xChangeMin = x;
    if (xChangeMax < x) xChangeMax = x;

    if (yChangeMin > y) yChangeMin = y;
    if (yChangeMax < y) yChangeMax = y;

    if (zChangeMin > z) zChangeMin = z;
    if (zChangeMax < z) zChangeMax = z;

    if (changes < MAX_CHANGES_BEFORE_RESEND) {
        short id = (short)((x << 12) | (z << 8) | (y));

        for (int i = 0; i < changes; i++) {
            if (changedTiles[i] == id) return;
        }

        changedTiles[changes++] = id;
    }
}



void PlayerChunkMap::PlayerChunk::prioritiseTileChanges() {
    prioritised = true;
}

void PlayerChunkMap::PlayerChunk::broadcast(std::shared_ptr<Packet> packet) {
    std::vector<std::shared_ptr<ServerPlayer> > sentTo;
    for (unsigned int i = 0; i < players.size(); i++) {
        std::shared_ptr<ServerPlayer> player = players[i];

        
        
        
        
        
        
        
        
        bool dontSend = false;
        if (sentTo.size()) {
            INetworkPlayer* thisPlayer = player->connection->getNetworkPlayer();
            if (thisPlayer == nullptr) {
                dontSend = true;
            } else {
                for (unsigned int j = 0; j < sentTo.size(); j++) {
                    std::shared_ptr<ServerPlayer> player2 = sentTo[j];
                    INetworkPlayer* otherPlayer =
                        player2->connection->getNetworkPlayer();
                    if (otherPlayer != nullptr &&
                        thisPlayer->IsSameSystem(otherPlayer)) {
                        dontSend = true;
                    }
                }
            }
        }
        if (dontSend) {
            continue;
        }

        
        
        
        
        
        int flagIndex =
            ServerPlayer::getFlagIndexForChunk(pos, parent->dimension);
        if (player->seenChunks.find(pos) != player->seenChunks.end() &&
            (player->connection->isLocal() ||
             g_NetworkManager.SystemFlagGet(
                 player->connection->getNetworkPlayer(), flagIndex))) {
            player->connection->send(packet);
            sentTo.push_back(player);
        }
    }
    
    
    
    

    
    
    
    
    
    if (!((packet->getId() == 51) || (packet->getId() == 52) ||
          (packet->getId() == 53))) {
        return;
    }

    for (int i = 0;
         i < parent->level->getServer()->getPlayers()->players.size(); i++) {
        std::shared_ptr<ServerPlayer> player =
            parent->level->getServer()->getPlayers()->players[i];
        
        
        if (player->connection == nullptr) continue;
        if (player->connection->isLocal()) continue;

        
        
        int flagIndex =
            ServerPlayer::getFlagIndexForChunk(pos, parent->dimension);
        if (!g_NetworkManager.SystemFlagGet(
                player->connection->getNetworkPlayer(), flagIndex))
            continue;

        
        
        bool dontSend = false;
        if (sentTo.size()) {
            INetworkPlayer* thisPlayer = player->connection->getNetworkPlayer();
            if (thisPlayer == nullptr) {
                dontSend = true;
            } else {
                for (unsigned int j = 0; j < sentTo.size(); j++) {
                    std::shared_ptr<ServerPlayer> player2 = sentTo[j];
                    INetworkPlayer* otherPlayer =
                        player2->connection->getNetworkPlayer();
                    if (otherPlayer != nullptr &&
                        thisPlayer->IsSameSystem(otherPlayer)) {
                        dontSend = true;
                    }
                }
            }
        }
        if (!dontSend) {
            player->connection->send(packet);
            sentTo.push_back(player);
        }
    }
}

bool PlayerChunkMap::PlayerChunk::broadcastChanges(bool allowRegionUpdate) {
    bool didRegionUpdate = false;
    ServerLevel* level = parent->getLevel();
    if (ticksToNextRegionUpdate > 0) ticksToNextRegionUpdate--;
    if (changes == 0) {
        prioritised = false;
        return false;
    }
    if (changes == 1) {
        int x = pos.x * 16 + xChangeMin;
        int y = yChangeMin;
        int z = pos.z * 16 + zChangeMin;
        broadcast(std::shared_ptr<TileUpdatePacket>(
            new TileUpdatePacket(x, y, z, level)));
        if (level->isEntityTile(x, y, z)) {
            broadcast(level->getTileEntity(x, y, z));
        }
    } else if (changes == MAX_CHANGES_BEFORE_RESEND) {
        
        if (!prioritised) {
            if (!allowRegionUpdate || (ticksToNextRegionUpdate > 0)) {
                return false;
            }
        }

        yChangeMin = yChangeMin / 2 * 2;
        yChangeMax = (yChangeMax / 2 + 1) * 2;
        int xp = xChangeMin + pos.x * 16;
        int yp = yChangeMin;
        int zp = zChangeMin + pos.z * 16;
        int xs = xChangeMax - xChangeMin + 1;
        int ys = yChangeMax - yChangeMin + 2;
        int zs = zChangeMax - zChangeMin + 1;

        
        
        
        
        if (ys > 256) ys = 256;

        broadcast(std::shared_ptr<BlockRegionUpdatePacket>(
            new BlockRegionUpdatePacket(xp, yp, zp, xs, ys, zs, level)));
        std::vector<std::shared_ptr<TileEntity> >* tes =
            level->getTileEntitiesInRegion(xp, yp, zp, xp + xs, yp + ys,
                                           zp + zs);
        for (unsigned int i = 0; i < tes->size(); i++) {
            broadcast(tes->at(i));
        }
        delete tes;
        ticksToNextRegionUpdate = MIN_TICKS_BETWEEN_REGION_UPDATE;
        didRegionUpdate = true;
    } else {
        
        
        
        broadcast(std::make_shared<ChunkTilesUpdatePacket>(
            pos.x, pos.z, changedTiles, (uint8_t)changes, level));
        for (int i = 0; i < changes; i++) {
            int x = pos.x * 16 + ((changedTiles[i] >> 12) & 15);
            int y = ((changedTiles[i]) & 255);
            int z = pos.z * 16 + ((changedTiles[i] >> 8) & 15);

            if (level->isEntityTile(x, y, z)) {
                
                broadcast(level->getTileEntity(x, y, z));
            }
        }
    }
    changes = 0;
    prioritised = false;
    return didRegionUpdate;
}

void PlayerChunkMap::PlayerChunk::broadcast(std::shared_ptr<TileEntity> te) {
    if (te != nullptr) {
        std::shared_ptr<Packet> p = te->getUpdatePacket();
        if (p != nullptr) {
            broadcast(p);
        }
    }
}

PlayerChunkMap::PlayerChunkMap(ServerLevel* level, int dimension, int radius) {
    assert(radius <= MAX_VIEW_DISTANCE);
    assert(radius >= MIN_VIEW_DISTANCE);
    this->radius = radius;
    this->level = level;
    this->dimension = dimension;
    lastInhabitedUpdate = 0;
}

PlayerChunkMap::~PlayerChunkMap() {
    for (auto it = chunks.begin(); it != chunks.end(); it++) {
        delete it->second;
    }
}

ServerLevel* PlayerChunkMap::getLevel() { return level; }

void PlayerChunkMap::tick() {
    int64_t time = level->getGameTime();

    if (time - lastInhabitedUpdate > Level::TICKS_PER_DAY / 3) {
        lastInhabitedUpdate = time;

        for (int i = 0; i < knownChunks.size(); i++) {
            PlayerChunk* chunk = knownChunks.at(i);

            
            
            

            chunk->updateInhabitedTime();
        }
    }

    
    
    
    bool regionUpdateSent = false;
    for (unsigned int i = 0; i < changedChunks.size();) {
        regionUpdateSent |=
            changedChunks[i]->broadcastChanges(!regionUpdateSent);
        
        
        if (changedChunks[i]->changes == 0) {
            changedChunks[i] = changedChunks.back();
            changedChunks.pop_back();
        } else {
            
            
            i++;
        }
    }

    for (unsigned int i = 0; i < players.size(); i++) {
        tickAddRequests(players[i]);
    }

    
    
    
    
    
    
    
    
    
}

bool PlayerChunkMap::hasChunk(int x, int z) {
    int64_t id = (x + 0x7fffffffLL) | ((z + 0x7fffffffLL) << 32);
    return chunks.find(id) != chunks.end();
}

PlayerChunkMap::PlayerChunk* PlayerChunkMap::getChunk(int x, int z,
                                                      bool create) {
    int64_t id = (x + 0x7fffffffLL) | ((z + 0x7fffffffLL) << 32);
    auto it = chunks.find(id);

    PlayerChunk* chunk = nullptr;
    if (it != chunks.end()) {
        chunk = it->second;
    } else if (create) {
        chunk = new PlayerChunk(x, z, this);
        chunks[id] = chunk;
        knownChunks.push_back(chunk);
    }

    return chunk;
}



void PlayerChunkMap::getChunkAndAddPlayer(
    int x, int z, std::shared_ptr<ServerPlayer> player) {
    int64_t id = (x + 0x7fffffffLL) | ((z + 0x7fffffffLL) << 32);
    auto it = chunks.find(id);

    if (it != chunks.end()) {
        it->second->add(player);
    } else {
        addRequests.push_back(PlayerChunkAddRequest(x, z, player));
    }
}



void PlayerChunkMap::getChunkAndRemovePlayer(
    int x, int z, std::shared_ptr<ServerPlayer> player) {
    for (auto it = addRequests.begin(); it != addRequests.end(); it++) {
        if ((it->x == x) && (it->z == z) && (it->player == player)) {
            addRequests.erase(it);
            return;
        }
    }
    int64_t id = (x + 0x7fffffffLL) | ((z + 0x7fffffffLL) << 32);
    auto it = chunks.find(id);

    if (it != chunks.end()) {
        it->second->remove(player);
    }
}



void PlayerChunkMap::tickAddRequests(std::shared_ptr<ServerPlayer> player) {
    if (addRequests.size()) {
        
        int px = (int)player->x;
        int pz = (int)player->z;
        int minDistSq = -1;

        auto itNearest = addRequests.end();
        for (auto it = addRequests.begin(); it != addRequests.end(); it++) {
            if (it->player == player) {
                int xm = (it->x * 16) + 8;
                int zm = (it->z * 16) + 8;
                int distSq = (xm - px) * (xm - px) + (zm - pz) * (zm - pz);
                if ((minDistSq == -1) || (distSq < minDistSq)) {
                    minDistSq = distSq;
                    itNearest = it;
                }
            }
        }

        
        if (itNearest != addRequests.end()) {
            getChunk(itNearest->x, itNearest->z, true)->add(itNearest->player);
            addRequests.erase(itNearest);
        }
    }
}

void PlayerChunkMap::broadcastTileUpdate(std::shared_ptr<Packet> packet, int x,
                                         int y, int z) {
    int xc = x >> 4;
    int zc = z >> 4;
    PlayerChunk* chunk = getChunk(xc, zc, false);
    if (chunk != nullptr) {
        chunk->broadcast(packet);
    }
}

void PlayerChunkMap::tileChanged(int x, int y, int z) {
    int xc = x >> 4;
    int zc = z >> 4;
    PlayerChunk* chunk = getChunk(xc, zc, false);
    if (chunk != nullptr) {
        chunk->tileChanged(x & 15, y, z & 15);
    }
}

bool PlayerChunkMap::isTrackingTile(int x, int y, int z) {
    int xc = x >> 4;
    int zc = z >> 4;
    PlayerChunk* chunk = getChunk(xc, zc, false);
    if (chunk) return true;
    return false;
}



void PlayerChunkMap::prioritiseTileChanges(int x, int y, int z) {
    int xc = x >> 4;
    int zc = z >> 4;
    PlayerChunk* chunk = getChunk(xc, zc, false);
    if (chunk != nullptr) {
        chunk->prioritiseTileChanges();
    }
}

void PlayerChunkMap::add(std::shared_ptr<ServerPlayer> player) {
    static int direction[4][2] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};

    int xc = (int)player->x >> 4;
    int zc = (int)player->z >> 4;

    player->lastMoveX = player->x;
    player->lastMoveZ = player->z;

    
    
    
    

    
    int facing = 0;
    int size = radius;
    int dx = 0;
    int dz = 0;

    
    getChunk(xc, zc, true)->add(player, false);

    
    
    int minX, maxX, minZ, maxZ;
    minX = maxX = xc;
    minZ = maxZ = zc;

    
    
    
    
    
    
    const int maxLegSizeToAddNow = 14;

    
    for (int legSize = 1; legSize <= size * 2; legSize++) {
        for (int leg = 0; leg < 2; leg++) {
            int* dir = direction[facing++ % 4];

            for (int k = 0; k < legSize; k++) {
                dx += dir[0];
                dz += dir[1];

                int targetX, targetZ;
                targetX = xc + dx;
                targetZ = zc + dz;

                if ((legSize < maxLegSizeToAddNow) ||
                    ((legSize == maxLegSizeToAddNow) &&
                     ((leg == 0) || (k < (legSize - 1))))) {
                    if (targetX > maxX) maxX = targetX;
                    if (targetX < minX) minX = targetX;
                    if (targetZ > maxZ) maxZ = targetZ;
                    if (targetZ < minZ) minZ = targetZ;

                    getChunk(targetX, targetZ, true)->add(player, false);
                } else {
                    getChunkAndAddPlayer(targetX, targetZ, player);
                }
            }
        }
    }

    
    facing %= 4;
    for (int k = 0; k < size * 2; k++) {
        dx += direction[facing][0];
        dz += direction[facing][1];

        int targetX, targetZ;
        targetX = xc + dx;
        targetZ = zc + dz;
        if ((size * 2) <= maxLegSizeToAddNow) {
            if (targetX > maxX) maxX = targetX;
            if (targetX < minX) minX = targetX;
            if (targetZ > maxZ) maxZ = targetZ;
            if (targetZ < minZ) minZ = targetZ;

            getChunk(targetX, targetZ, true)->add(player, false);
        } else {
            getChunkAndAddPlayer(targetX, targetZ, player);
        }
    }
    

    player->connection->send(std::shared_ptr<ChunkVisibilityAreaPacket>(
        new ChunkVisibilityAreaPacket(minX, maxX, minZ, maxZ)));

#ifdef _LARGE_WORLDS
    getLevel()->cache->dontDrop(xc, zc);
#endif

    players.push_back(player);
}

void PlayerChunkMap::remove(std::shared_ptr<ServerPlayer> player) {
    int xc = ((int)player->lastMoveX) >> 4;
    int zc = ((int)player->lastMoveZ) >> 4;

    for (int x = xc - radius; x <= xc + radius; x++)
        for (int z = zc - radius; z <= zc + radius; z++) {
            PlayerChunk* playerChunk = getChunk(x, z, false);
            if (playerChunk != nullptr) playerChunk->remove(player);
        }

    auto it = find(players.begin(), players.end(), player);
    if (players.size() > 0 && it != players.end())
        players.erase(find(players.begin(), players.end(), player));

    
    
    for (auto it = addRequests.begin(); it != addRequests.end();) {
        if (it->player == player) {
            it = addRequests.erase(it);
        } else {
            ++it;
        }
    }
}

bool PlayerChunkMap::chunkInRange(int x, int z, int xc, int zc) {
    
    int xd = x - xc;
    int zd = z - zc;
    if (xd < -radius || xd > radius) return false;
    if (zd < -radius || zd > radius) return false;
    return true;
}




void PlayerChunkMap::move(std::shared_ptr<ServerPlayer> player) {
    int xc = ((int)player->x) >> 4;
    int zc = ((int)player->z) >> 4;

    double _xd = player->lastMoveX - player->x;
    double _zd = player->lastMoveZ - player->z;
    double dist = _xd * _xd + _zd * _zd;
    if (dist < 8 * 8) return;

    int last_xc = ((int)player->lastMoveX) >> 4;
    int last_zc = ((int)player->lastMoveZ) >> 4;

    int xd = xc - last_xc;
    int zd = zc - last_zc;
    if (xd == 0 && zd == 0) return;

    for (int x = xc - radius; x <= xc + radius; x++)
        for (int z = zc - radius; z <= zc + radius; z++) {
            if (!chunkInRange(x, z, last_xc, last_zc)) {
                
                
                getChunkAndAddPlayer(x, z, player);
            }

            if (!chunkInRange(x - xd, z - zd, xc, zc)) {
                
                
                getChunkAndRemovePlayer(x - xd, z - zd, player);
            }
        }

    
    
    
    
    
    
    
    
    int pvd = player->getViewDistance();
    if (pvd < radius) {
        for (int x = xc - radius; x <= xc + radius; x++)
            for (int z = zc - radius; z <= zc + radius; z++) {
                int dcx = x - xc;
                if (dcx < 0) dcx = -dcx;
                int dcz = z - zc;
                if (dcz < 0) dcz = -dcz;
                if (dcx > pvd || dcz > pvd) {
                    getChunkAndRemovePlayer(x, z, player);
                }
            }
    }

    player->lastMoveX = player->x;
    player->lastMoveZ = player->z;
}

int PlayerChunkMap::getMaxRange() { return radius * 16 - 16; }

bool PlayerChunkMap::isPlayerIn(std::shared_ptr<ServerPlayer> player,
                                int xChunk, int zChunk) {
    PlayerChunk* chunk = getChunk(xChunk, zChunk, false);

    if (chunk == nullptr) {
        return false;
    } else {
        auto it1 = find(chunk->players.begin(), chunk->players.end(), player);
        auto it2 = find(player->chunksToSend.begin(),
                        player->chunksToSend.end(), chunk->pos);
        return it1 != chunk->players.end() && it2 == player->chunksToSend.end();
    }

    
    
}

int PlayerChunkMap::convertChunkRangeToBlock(int radius) {
    return radius * 16 - 16;
}


void PlayerChunkMap::setRadius(int newRadius) {
    if (radius != newRadius) {
        PlayerList* players = level->getServer()->getPlayerList();
        for (int i = 0; i < players->players.size(); i += 1) {
            std::shared_ptr<ServerPlayer> player = players->players[i];
            if (player->level == level) {
                int xc = ((int)player->x) >> 4;
                int zc = ((int)player->z) >> 4;

                for (int x = xc - newRadius; x <= xc + newRadius; x++)
                    for (int z = zc - newRadius; z <= zc + newRadius; z++) {
                        
                        if (x < xc - radius || x > xc + radius ||
                            z < zc - radius || z > zc + radius) {
                            getChunkAndAddPlayer(x, z, player);
                        }
                    }
            }
        }

        assert(radius <= MAX_VIEW_DISTANCE);
        assert(radius >= MIN_VIEW_DISTANCE);
        this->radius = newRadius;
    }
}







void PlayerChunkMap::adjustPlayerViewDistance(
    std::shared_ptr<ServerPlayer> player, int oldChunks, int newChunks) {
    
    
    
    auto bound = [this](int chunks) {
        if (chunks < MIN_VIEW_DISTANCE) return MIN_VIEW_DISTANCE;
        if (chunks > radius) return radius;
        return chunks;
    };
    oldChunks = bound(oldChunks);
    newChunks = bound(newChunks);

    if (newChunks == oldChunks) return;

    int xc = ((int)player->x) >> 4;
    int zc = ((int)player->z) >> 4;

    if (newChunks > oldChunks) {
        
        for (int x = xc - newChunks; x <= xc + newChunks; x++)
            for (int z = zc - newChunks; z <= zc + newChunks; z++) {
                if (x < xc - oldChunks || x > xc + oldChunks ||
                    z < zc - oldChunks || z > zc + oldChunks) {
                    getChunkAndAddPlayer(x, z, player);
                }
            }
    } else {
        
        for (int x = xc - oldChunks; x <= xc + oldChunks; x++)
            for (int z = zc - oldChunks; z <= zc + oldChunks; z++) {
                if (x < xc - newChunks || x > xc + newChunks ||
                    z < zc - newChunks || z > zc + newChunks) {
                    getChunkAndRemovePlayer(x, z, player);
                }
            }
    }
}