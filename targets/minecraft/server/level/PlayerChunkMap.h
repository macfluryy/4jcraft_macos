#pragma once
#include <stdint.h>

#include <memory>
#include <unordered_map>
#include <vector>

#include "java/JavaIntHash.h"
#include "minecraft/world/level/ChunkPos.h"

class ServerPlayer;
class ServerLevel;
class MinecraftServer;
class Packet;
class TileEntity;
class LevelChunk;

class PlayerChunkMap {
public:
#ifdef _LARGE_WORLDS
    static const int MAX_VIEW_DISTANCE = 30;
#else
    static const int MAX_VIEW_DISTANCE = 15;
#endif
    static const int MIN_VIEW_DISTANCE = 3;
    static const int MAX_CHANGES_BEFORE_RESEND = 10;
    static const int MIN_TICKS_BETWEEN_REGION_UPDATE = 10;

    // 4J - added
    class PlayerChunkAddRequest {
    public:
        int x, z;
        std::shared_ptr<ServerPlayer> player;
        PlayerChunkAddRequest(int x, int z,
                              std::shared_ptr<ServerPlayer> player)
            : x(x), z(z), player(player) {}
    };

    class PlayerChunk {
        friend class PlayerChunkMap;

    private:
        PlayerChunkMap* parent;  // 4J added
        std::vector<std::shared_ptr<ServerPlayer> > players;
        // int x, z;
        ChunkPos pos;

        std::vector<short> changedTiles;
        int changes;
        int xChangeMin, xChangeMax;
        int yChangeMin, yChangeMax;
        int zChangeMin, zChangeMax;
        int ticksToNextRegionUpdate;  // 4J added
        bool prioritised;             // 4J added
        int64_t firstInhabitedTime;

    public:
        PlayerChunk(int x, int z, PlayerChunkMap* pcm);
        ~PlayerChunk();

        // 4J Added sendPacket param so we can aggregate the initial send into
        // one much smaller packet
        void add(std::shared_ptr<ServerPlayer> player, bool sendPacket = true);
        void remove(std::shared_ptr<ServerPlayer> player);
        void updateInhabitedTime();

    private:
        void updateInhabitedTime(LevelChunk* chunk);

    public:
        void tileChanged(int x, int y, int z);
        void prioritiseTileChanges();  // 4J added
        void broadcast(std::shared_ptr<Packet> packet);
        bool broadcastChanges(bool allowRegionUpdate);  // 4J - added parm

    private:
        void broadcast(std::shared_ptr<TileEntity> te);
    };

public:
    std::vector<std::shared_ptr<ServerPlayer> > players;
    void flagEntitiesToBeRemoved(unsigned int* flags,
                                 bool* removedFound);  // 4J added
private:
    std::unordered_map<int64_t, PlayerChunk*, LongKeyHash, LongKeyEq>
        chunks;  // 4J - was LongHashMap
    std::vector<PlayerChunk*> changedChunks;
    std::vector<PlayerChunk*> knownChunks;
    std::vector<PlayerChunkAddRequest> addRequests;              // 4J added
    void tickAddRequests(std::shared_ptr<ServerPlayer> player);  // 4J added

    ServerLevel* level;
    int radius;
    int dimension;
    int64_t lastInhabitedUpdate;

public:
    PlayerChunkMap(ServerLevel* level, int dimension, int radius);
    ~PlayerChunkMap();
    ServerLevel* getLevel();
    void tick();
    bool hasChunk(int x, int z);

private:
    PlayerChunk* getChunk(int x, int z, bool create);
    void getChunkAndAddPlayer(
        int x, int z, std::shared_ptr<ServerPlayer> player);  // 4J added
    void getChunkAndRemovePlayer(
        int x, int z, std::shared_ptr<ServerPlayer> player);  // 4J added
public:
    void broadcastTileUpdate(std::shared_ptr<Packet> packet, int x, int y,
                             int z);
    void tileChanged(int x, int y, int z);
    bool isTrackingTile(int x, int y, int z);         // 4J added
    void prioritiseTileChanges(int x, int y, int z);  // 4J added
    void add(std::shared_ptr<ServerPlayer> player);
    void remove(std::shared_ptr<ServerPlayer> player);

private:
    bool chunkInRange(int x, int z, int xc, int zc);

public:
    void move(std::shared_ptr<ServerPlayer> player);
    int getMaxRange();
    bool isPlayerIn(std::shared_ptr<ServerPlayer> player, int xChunk,
                    int zChunk);
    static int convertChunkRangeToBlock(int radius);

    // AP added for Vita
    void setRadius(int newRadius);

    // 4J macOS - adjust a SINGLE player's effective view-distance (in chunks)
    // around their current chunk, square-radius model matching add()/move()/
    // setRadius(). Both oldChunks and newChunks are bounded to
    // [MIN_VIEW_DISTANCE, this->radius] (a per-player view distance can never
    // exceed the server-level radius the map maintains subscriptions for, nor
    // drop below MIN_VIEW_DISTANCE). On increase, newly-in-range chunks are
    // subscribed via getChunkAndAddPlayer; on decrease, now-excess chunks are
    // unsubscribed via getChunkAndRemovePlayer; equal is a no-op.
    // MUST be called from the server tick only (not the network thread).
    void adjustPlayerViewDistance(std::shared_ptr<ServerPlayer> player,
                                  int oldChunks, int newChunks);
};
