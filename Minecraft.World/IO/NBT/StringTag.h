#pragma once
#include "Tag.h"

class StringTag : public Tag {
public:
    std::wstring data;
    StringTag(const std::wstring& name) : Tag(name) {}
    StringTag(const std::wstring& name, const std::wstring& data) : Tag(name) {
        this->data = data;
    }

    void write(DataOutput* dos) { dos->writeUTF(data); }

    void load(DataInput* dis) { data = dis->readUTF(); }

    uint8_t getId() { return TAG_String; }

    std::wstring toString() { return data; }

    Tag* copy() { return new StringTag(getName(), data); }

    bool equals(Tag* obj) {
        if (Tag::equals(obj)) {
            StringTag* o = (StringTag*)obj;
            return ((data.empty() && o->data.empty()) ||
                    (!data.empty() && data.compare(o->data) == 0));
        }
        return false;
    }
};
