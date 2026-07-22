#pragma once

#include <stdint.h>

#include <format>
#include <memory>
#include <string>
#include <vector>

#include "minecraft/world/item/ItemInstance.h"

class Pos;
class DataInputStream;
class DataOutputStream;

class SynchedEntityData {
public:
    class DataItem {
        friend class SynchedEntityData;

    private:
        const int type;
        const int id;
        
        
        union {
            uint8_t value_byte;
            int value_int;
            short value_short;
            float value_float;
        };
        std::wstring value_wstring;
        std::shared_ptr<ItemInstance> value_itemInstance;
        bool dirty;

    public:
        
        
        DataItem(int type, int id, uint8_t value);
        DataItem(int type, int id, int value);
        DataItem(int type, int id, const std::wstring& value);
        DataItem(int type, int id, std::shared_ptr<ItemInstance> itemInstance);
        DataItem(int type, int id, short value);
        DataItem(int type, int id, float value);

        int getId();
        void setValue(uint8_t value);
        void setValue(int value);
        void setValue(short value);
        void setValue(float value);
        void setValue(const std::wstring& value);
        void setValue(std::shared_ptr<ItemInstance> value);
        uint8_t getValue_byte();
        int getValue_int();
        short getValue_short();
        float getValue_float();
        std::wstring getValue_wstring();
        std::shared_ptr<ItemInstance> getValue_itemInstance();
        int getType();
        bool isDirty();
        void setDirty(bool dirty);
    };

public:
    static const int MAX_STRING_DATA_LENGTH = 64;
    static const int EOF_MARKER = 0x7f;

    static const int TYPE_BYTE = 0;
    static const int TYPE_SHORT = 1;
    static const int TYPE_INT = 2;
    static const int TYPE_FLOAT = 3;
    static const int TYPE_STRING = 4;
    
    static const int TYPE_ITEMINSTANCE = 5;
    static const int TYPE_POS = 6;

private:
    bool m_isEmpty;

    
private:
    static const int TYPE_MASK = 0xe0;
    static const int TYPE_SHIFT = 5;

    
    static const int MAX_ID_VALUE = ~TYPE_MASK & 0xff;

    std::shared_ptr<DataItem> itemsById[MAX_ID_VALUE + 1];
    bool m_isDirty;

public:
    SynchedEntityData();

    
    
    
    
    void define(int id, uint8_t value);
    void define(int id, const std::wstring& value);
    void define(int id, int value);
    void define(int id, short value);
    void define(int id, float value);
    void defineNULL(int id, void* pVal);

    void checkId(int id);  
                           
    uint8_t getByte(int id);
    short getShort(int id);
    int getInteger(int id);
    float getFloat(int id);
    std::wstring getString(int id);
    std::shared_ptr<ItemInstance> getItemInstance(int id);
    Pos* getPos(int id);
    
    void set(int id, uint8_t value);
    void set(int id, int value);
    void set(int id, short value);
    void set(int id, float value);
    void set(int id, const std::wstring& value);
    void set(int id, std::shared_ptr<ItemInstance>);
    void markDirty(int id);
    bool isDirty();
    static void pack(std::vector<std::shared_ptr<DataItem> >* items,
                     DataOutputStream* output);  
    std::vector<std::shared_ptr<DataItem> >* packDirty();
    void packAll(DataOutputStream* output);  
    std::vector<std::shared_ptr<DataItem> >* getAll();

private:
    static void writeDataItem(
        DataOutputStream* output,
        std::shared_ptr<DataItem> dataItem);  

public:
    static std::vector<std::shared_ptr<DataItem> >* unpack(
        DataInputStream* input);  

    




public:
    void assignValues(std::vector<std::shared_ptr<DataItem> >* items);
    bool isEmpty();
    void clearDirty();

    
    int getSizeInBytes();
};