#pragma once

#include <stdint.h>

#include <memory>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "nbt/CompoundTag.h"

class Packet;
class PacketListener;
class DataInputStream;
class DataOutputStream;
class CompoundTag;

#define PACKET_ENABLE_STAT_TRACKING 0

class Packet;
class ItemInstance;

typedef std::shared_ptr<Packet> (*packetCreateFn)();

class Packet {
public:
    class PacketStatistics {
    private:
        int count;
        int totalSize;

        static const int TOTAL_TICKS = 100;

        
        int64_t countSamples[TOTAL_TICKS];
        int64_t sizeSamples[TOTAL_TICKS];
        int64_t timeSamples[TOTAL_TICKS];
        int samplesPos;

    public:
        const int id;

    public:
        PacketStatistics(int id);
        void addPacket(int bytes);
        int getCount();
        int getTotalSize();
        double getAverageSize();
        int64_t getRunningTotal();
        int64_t getRunningCount();
        void IncrementPos();
    };

    
    static void staticCtor();

public:
    static std::unordered_map<int, packetCreateFn>
        idToCreateMap;  
                        

    static std::unordered_set<int> clientReceivedPackets;
    static std::unordered_set<int> serverReceivedPackets;
    static std::unordered_set<int> sendToAnyClientPackets;

    
    
    
    static void map(int id, bool receiveOnClient, bool receiveOnServer,
                    bool sendToAnyClient, bool renderStats,
                    const std::type_info& clazz, packetCreateFn);

public:
    const int64_t createTime;

    Packet();

    static std::shared_ptr<Packet> getPacket(int id);

    
    static bool canSendToAnyClient(std::shared_ptr<Packet> packet);

    static void writeBytes(DataOutputStream* dataoutputstream,
                           const std::vector<uint8_t>& bytes);
    static std::vector<uint8_t> readBytes(DataInputStream* datainputstream);

    virtual int getId() = 0;

    bool shouldDelay;

private:
    
    static std::unordered_map<int, PacketStatistics*>
        outgoingStatistics;  
    static std::vector<PacketStatistics*> renderableStats;
    static int renderPos;

public:
    static void recordOutgoingPacket(std::shared_ptr<Packet> packet,
                                     int playerIndex);
    static void updatePacketStatsPIX();

private:
    static std::unordered_map<int, PacketStatistics*> statistics;
    

public:
    static std::shared_ptr<Packet> readPacket(DataInputStream* dis,
                                              bool isServer);
    static void writePacket(std::shared_ptr<Packet> packet,
                            DataOutputStream* dos);
    static void writeUtf(const std::wstring& value, DataOutputStream* dos);
    static std::wstring readUtf(DataInputStream* dis, int maxLength);
    virtual void read(
        DataInputStream* dis) = 0;  
                                    
    virtual void write(
        DataOutputStream* dos) = 0;  
                                     
    virtual void handle(PacketListener* listener) = 0;
    virtual int getEstimatedSize() = 0;
    virtual bool canBeInvalidated();
    virtual bool isInvalidatedBy(std::shared_ptr<Packet> packet);
    virtual bool isAync();

    
    static std::shared_ptr<ItemInstance> readItem(DataInputStream* dis);
    static void writeItem(std::shared_ptr<ItemInstance> item,
                          DataOutputStream* dos);
    static CompoundTag* readNbt(DataInputStream* dis);

protected:
    static void writeNbt(CompoundTag* tag, DataOutputStream* dos);
};