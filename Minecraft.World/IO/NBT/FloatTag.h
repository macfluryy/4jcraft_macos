#pragma once
#include "../Streams/InputOutputStream.h"
#include "Tag.h"

class FloatTag : public Tag {
public:
    float data;
    FloatTag(const std::wstring& name) : Tag(name) {}
    FloatTag(const std::wstring& name, float data) : Tag(name) {
        this->data = data;
    }

    void write(DataOutput* dos) { dos->writeFloat(data); }
    void load(DataInput* dis, int tagDepth) { data = dis->readFloat(); }

    uint8_t getId() { return TAG_Float; }
    std::wstring toString() {
        static wchar_t buf[32];
        swprintf(buf, 32, L"%f", data);
        return std::wstring(buf);
    }

    Tag* copy() { return new FloatTag(getName(), data); }

    bool equals(Tag* obj) {
        if (Tag::equals(obj)) {
            FloatTag* o = (FloatTag*)obj;
            return data == o->data;
        }
        return false;
    }
};