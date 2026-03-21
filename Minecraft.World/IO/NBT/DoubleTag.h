#pragma once
#include "../Streams/InputOutputStream.h"
#include "Tag.h"

class DoubleTag : public Tag {
public:
    double data;
    DoubleTag(const std::wstring& name) : Tag(name) {}
    DoubleTag(const std::wstring& name, double data) : Tag(name) {
        this->data = data;
    }

    void write(DataOutput* dos) { dos->writeDouble(data); }
    void load(DataInput* dis, int tagDepth) { data = dis->readDouble(); }

    uint8_t getId() { return TAG_Double; }
    std::wstring toString() {
        static wchar_t buf[32];
        swprintf(buf, 32, L"%f", data);
        return std::wstring(buf);
    }

    Tag* copy() { return new DoubleTag(getName(), data); }

    bool equals(Tag* obj) {
        if (Tag::equals(obj)) {
            DoubleTag* o = (DoubleTag*)obj;
            return data == o->data;
        }
        return false;
    }
};