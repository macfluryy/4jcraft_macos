#pragma once
#include "Tag.h"

class EndTag : public Tag {
public:
    EndTag() : Tag(L"") {}
    EndTag(const std::wstring& name) : Tag(name) {}

    void load(DataInput* dis) {};
    void write(DataOutput* dos) {};

    uint8_t getId() { return TAG_End; }
    std::wstring toString() { return std::wstring(L"END"); }

    Tag* copy() { return new EndTag(); }

    bool equals(Tag* obj) { return Tag::equals(obj); }
};