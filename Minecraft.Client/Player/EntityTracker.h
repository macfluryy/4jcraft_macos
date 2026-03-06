#pragma once
#include "../../Minecraft.World/Util/HashExtension.h"
#include "../../Minecraft.World/Util/JavaIntHash.h"
class Entity;
class ServerPlayer;
class TrackedEntity;
class MinecraftServer;
class Packet;

using namespace std;

class EntityTracker
{
private:
	ServerLevel *level;
	unordered_set<std::shared_ptr<TrackedEntity> > entities;
    unordered_map<int, std::shared_ptr<TrackedEntity> , IntKeyHash2, IntKeyEq> entityMap;	// was IntHashMap
    int maxRange;

public:
	EntityTracker(ServerLevel *level);
    void addEntity(std::shared_ptr<Entity> e);
    void addEntity(std::shared_ptr<Entity> e, int range, int updateInterval);
    void addEntity(std::shared_ptr<Entity> e, int range, int updateInterval, bool trackDeltas);
    void removeEntity(std::shared_ptr<Entity> e);
	void removePlayer(std::shared_ptr<Entity> e);	// 4J added
    void tick();
    void broadcast(std::shared_ptr<Entity> e, std::shared_ptr<Packet> packet);
    void broadcastAndSend(std::shared_ptr<Entity> e, std::shared_ptr<Packet> packet);
    void clear(std::shared_ptr<ServerPlayer> serverPlayer);
	void updateMaxRange();	// AP added for Vita


	// 4J-JEV: Added, needed access to tracked entity of a riders mount.
	std::shared_ptr<TrackedEntity> getTracker(std::shared_ptr<Entity> entity);
};
