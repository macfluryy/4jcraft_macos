#pragma once
#include "Tag.h"

class ByteTag : public Tag {
public:
    uint8_t data;
    ByteTag(const std::wstring& name) : Tag(name) {}
    ByteTag(const std::wstring& name, uint8_t data) : Tag(name) {
        this->data = data;
    }

    void write(DataOutput* dos) { dos->writeByte(data); }
    void load(DataInput* dis) { data = dis->readByte(); }

    uint8_t getId() { return TAG_Byte; }
    std::wstring toString() {
        static wchar_t buf[32];
        swprintf(buf, 32, L"%d", data);
        return std::wstring(buf);
    }

    bool equals(Tag* obj) {
        if (Tag::equals(obj)) {
            ByteTag* o = (ByteTag*)obj;
            return data == o->data;
        }
        return false;
    }

    Tag* copy() { return new ByteTag(getName(), data); }
};