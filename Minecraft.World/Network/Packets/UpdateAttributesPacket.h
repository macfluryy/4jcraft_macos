#pragma once

#include "Packet.h"

class AttributeModifier;
class AttributeInstance;

class UpdateAttributesPacket
    : public Packet,
      public std::enable_shared_from_this<UpdateAttributesPacket> {
public:
    class AttributeSnapshot {
    private:
        eATTRIBUTE_ID id;
        double base;
        std::unordered_set<AttributeModifier*> modifiers;

    public:
        AttributeSnapshot(eATTRIBUTE_ID id, double base,
                          std::unordered_set<AttributeModifier*>* modifiers);
        ~AttributeSnapshot();

        eATTRIBUTE_ID getId();
        double getBase();
        std::unordered_set<AttributeModifier*>* getModifiers();
    };

private:
    int entityId;
    std::unordered_set<AttributeSnapshot*> attributes;

public:
    UpdateAttributesPacket();
    UpdateAttributesPacket(int entityId,
                           std::unordered_set<AttributeInstance*>* values);
    ~UpdateAttributesPacket();

    void read(DataInputStream* dis);
    void write(DataOutputStream* dos);
    void handle(PacketListener* listener);
    int getEstimatedSize();
    int getEntityId();
    std::unordered_set<AttributeSnapshot*> getValues();

public:
    static std::shared_ptr<Packet> create() {
        return std::shared_ptr<Packet>(new UpdateAttributesPacket());
    }
    virtual int getId() { return 44; }
};