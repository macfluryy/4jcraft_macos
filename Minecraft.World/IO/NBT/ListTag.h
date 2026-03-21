#pragma once

#include "Tag.h"

template <class T>
class ListTag : public Tag {
private:
    std::vector<Tag*> list;
    uint8_t type;

public:
    ListTag() : Tag(L"") {}
    ListTag(const std::wstring& name) : Tag(name) {}

    void write(DataOutput* dos) {
        if (list.size() > 0)
            type = (list[0])->getId();
        else
            type = static_cast<uint8_t>(1);

        dos->writeByte(type);
        dos->writeInt((int)list.size());

        AUTO_VAR(itEnd, list.end());
        for (AUTO_VAR(it, list.begin()); it != itEnd; it++) (*it)->write(dos);
    }

    void load(DataInput* dis) {
        type = dis->readByte();
        int size = dis->readInt();

        list.clear();
        for (int i = 0; i < size; i++) {
            Tag* tag = Tag::newTag(type, L"");
            tag->load(dis);
            list.push_back(tag);
        }
    }

    uint8_t getId() { return TAG_List; }

    std::wstring toString() {
        static wchar_t buf[64];
        swprintf(buf, 64, L"%d entries of type %ls", list.size(),
                 Tag::getTagName(type));
        return std::wstring(buf);
    }

    void print(char* prefix, std::ostream out) {
        Tag::print(prefix, out);

        out << prefix << "{" << std::endl;

        char* newPrefix = new char[strlen(prefix) + 4];
        strcpy(newPrefix, prefix);
        strcat(newPrefix, "   ");
        AUTO_VAR(itEnd, list.end());
        for (AUTO_VAR(it, list.begin()); it != itEnd; it++) {
            (*it)->print(newPrefix, out);
        }
        delete[] newPrefix;
        out << prefix << "}" << std::endl;
    }

    void add(T* tag) {
        type = tag->getId();
        // 4J: List tag write/load doesn't preserve tag names so remove them so
        // we can safely do comparisons. This is the least invasive fix.
        tag->setName(L"");
        list.push_back(tag);
    }

    T* get(int index) { return (T*)list[index]; }

    int size() { return (int)list.size(); }

    virtual ~ListTag() {
        AUTO_VAR(itEnd, list.end());
        for (AUTO_VAR(it, list.begin()); it != itEnd; it++) {
            delete *it;
        }
    }

    virtual Tag* copy() {
        ListTag<T>* res = new ListTag<T>(getName());
        res->type = type;
        AUTO_VAR(itEnd, list.end());
        for (AUTO_VAR(it, list.begin()); it != itEnd; it++) {
            T* copy = (T*)(*it)->copy();
            res->list.push_back(copy);
        }
        return res;
    }

    virtual bool equals(Tag* obj) {
        if (Tag::equals(obj)) {
            ListTag* o = (ListTag*)obj;
            if (type == o->type) {
                bool equal = false;
                if (list.size() == o->list.size()) {
                    equal = true;
                    AUTO_VAR(itEnd, list.end());
                    // 4J Stu - Pretty inefficient method, but acceptable for
                    // how small and infrequent these comparisons are.
                    for (AUTO_VAR(it, list.begin()); it != itEnd; ++it) {
                        bool thisMatches = false;
                        for (AUTO_VAR(it2, o->list.begin());
                             it2 != o->list.end(); ++it2) {
                            if ((*it)->equals(*it2)) {
                                thisMatches = true;
                                break;
                            }
                        }
                        if (!thisMatches) {
                            equal = false;
                            break;
                        }
                    }
                }

                return equal;
            }
        }
        return false;
    }
};