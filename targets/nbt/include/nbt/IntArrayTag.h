#pragma once

#include "Tag.h"
#include "java/System.h"

class IntArrayTag : public Tag {
public:
    std::vector<int> data;

    IntArrayTag(const std::wstring& name) : Tag(name) {
        data = std::vector<int>();
    }

    IntArrayTag(const std::wstring& name, const std::vector<int>& data)
        : Tag(name) {
        this->data = data;
    }

    void write(DataOutput* dos) {
        dos->writeInt(data.size());
        for (unsigned int i = 0; i < data.size(); i++) {
            dos->writeInt(data[i]);
        }
    }

    void load(DataInput* dis, int tagDepth) {
        int length = dis->readInt();

        data = std::vector<int>(length);
        for (int i = 0; i < length; i++) {
            data[i] = dis->readInt();
        }
    }

    uint8_t getId() { return TAG_Int_Array; }

    std::wstring toString() {
        static wchar_t buf[32];
        swprintf(buf, 32, L"[%d bytes]", data.size());
        return std::wstring(buf);
    }

    bool equals(Tag* obj) {
        if (Tag::equals(obj)) {
            IntArrayTag* o = (IntArrayTag*)obj;
            return ((data.empty() && o->data.empty()) ||
                    (!data.empty() && data.size() == o->data.size() &&
                     memcmp(data.data(), o->data.data(),
                            data.size() * sizeof(int)) == 0));
        }
        return false;
    }

    Tag* copy() {
        std::vector<int> cp(data.size());
        std::copy(data.begin(), data.end(), cp.begin());
        return new IntArrayTag(getName(), cp);
    }
};