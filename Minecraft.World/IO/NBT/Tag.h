#pragma once
#include <ostream>
#include "../Streams/InputOutputStream.h"

class Tag {
public:
    static const uint8_t TAG_End = static_cast<uint8_t>(0);
    static const uint8_t TAG_Byte = static_cast<uint8_t>(1);
    static const uint8_t TAG_Short = static_cast<uint8_t>(2);
    static const uint8_t TAG_Int = static_cast<uint8_t>(3);
    static const uint8_t TAG_Long = static_cast<uint8_t>(4);
    static const uint8_t TAG_Float = static_cast<uint8_t>(5);
    static const uint8_t TAG_Double = static_cast<uint8_t>(6);
    static const uint8_t TAG_Byte_Array = static_cast<uint8_t>(7);
    static const uint8_t TAG_String = static_cast<uint8_t>(8);
    static const uint8_t TAG_List = static_cast<uint8_t>(9);
    static const uint8_t TAG_Compound = static_cast<uint8_t>(10);
    static const uint8_t TAG_Int_Array = static_cast<uint8_t>(11);

private:
    std::wstring name;

protected:
    Tag(const std::wstring& name);

public:
    virtual void write(DataOutput* dos) = 0;
    virtual void load(DataInput* dis) = 0;
    virtual std::wstring toString() = 0;
    virtual uint8_t getId() = 0;
    void print(std::ostream out);
    void print(char* prefix, std::wostream out);
    std::wstring getName();
    Tag* setName(const std::wstring& name);
    static Tag* readNamedTag(DataInput* dis);
    static void writeNamedTag(Tag* tag, DataOutput* dos);
    static Tag* newTag(uint8_t type, const std::wstring& name);
    static const wchar_t* getTagName(uint8_t type);
    virtual ~Tag() {}
    virtual bool equals(Tag* obj);  // 4J Brought forward from 1.2
    virtual Tag* copy() = 0;        // 4J Brought foward from 1.2
};
