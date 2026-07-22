#pragma once
class AttributeModifier;

#include <unordered_map>



enum eATTRIBUTE_ID {
    
    eAttributeId_GENERIC_MAXHEALTH,
    eAttributeId_GENERIC_FOLLOWRANGE,
    eAttributeId_GENERIC_KNOCKBACKRESISTANCE,
    eAttributeId_GENERIC_MOVEMENTSPEED,
    eAttributeId_GENERIC_ATTACKDAMAGE,
    eAttributeId_HORSE_JUMPSTRENGTH,
    eAttributeId_ZOMBIE_SPAWNREINFORCEMENTS,

    
    

    eAttributeId_COUNT
};

class Attribute {
public:
    static const int MAX_NAME_LENGTH = 64;

    





    virtual eATTRIBUTE_ID getId() = 0;

    







    virtual double sanitizeValue(double value) = 0;

    




    virtual double getDefaultValue() = 0;

    







    virtual bool isClientSyncable() = 0;

    
    static int getName(eATTRIBUTE_ID id);

protected:
    static const int AttributeNames[];
};

typedef std::unordered_map<eATTRIBUTE_ID, AttributeModifier*> attrAttrModMap;
