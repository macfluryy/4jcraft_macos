#pragma once
#include "Tag.h"

class IntTag : public Tag {
public:
    int data;
    IntTag(const std::wstring& name) : Tag(name) {}
    IntTag(const std::wstring& name, int data) : Tag(name) {
        this->data = data;
    }

    void write(DataOutput* dos) { dos->writeInt(data); }
    void load(DataInput* dis) { data = dis->readInt(); }

    uint8_t getId() { return TAG_Int; }
    std::wstring toString() {
        static wchar_t buf[32];
        swprintf(buf, 32, L"%d", data);
        return std::wstring(buf);
    }

    Tag* copy() { return new IntTag(getName(), data); }

    bool equals(Tag* obj) {
        if (Tag::equals(obj)) {
            IntTag* o = (IntTag*)obj;
            return data == o->data;
        }
        return false;
    }
};