#pragma once


#include <stdint.h>

#include <memory>
#include <string>
#include <unordered_map>

class CompoundTag;
class GameRuleDefinition;
class Connection;
class DataInputStream;
class DataOutputStream;
class ItemInstance;


class GameRule {
public:
    typedef struct _ValueType {
        union {
            int64_t i64;
            int i;
            char c;
            bool b;
            float f;
            double d;
            GameRule* gr;
        };
        bool isPointer;

        _ValueType() {
            i64 = 0;
            isPointer = false;
        }
    } ValueType;

private:
    GameRuleDefinition* m_definition;
    Connection* m_connection;

public:
    typedef std::unordered_map<std::wstring, ValueType> stringValueMapType;
    stringValueMapType m_parameters;  
                                      

public:
    GameRule(GameRuleDefinition* definition, Connection* connection = nullptr);
    virtual ~GameRule();

    Connection* getConnection() { return m_connection; }

    ValueType getParameter(const std::wstring& parameterName);
    void setParameter(const std::wstring& parameterName, ValueType value);
    GameRuleDefinition* getGameRuleDefinition();

    
    void onUseTile(int tileId, int x, int y, int z);
    void onCollectItem(std::shared_ptr<ItemInstance> item);

    
    
    
    

    void write(DataOutputStream* dos);
    void read(DataInputStream* dos);
};