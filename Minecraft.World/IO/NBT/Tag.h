#pragma once
#include <ostream>
#include "../Streams/InputOutputStream.h"
using namespace std;

 
class Tag
{
public:
    static const byte TAG_End = static_cast<byte>(0);
    static const byte TAG_Byte = static_cast<byte>(1);
    static const byte TAG_Short = static_cast<byte>(2);
    static const byte TAG_Int = static_cast<byte>(3);
    static const byte TAG_Long = static_cast<byte>(4);
    static const byte TAG_Float = static_cast<byte>(5);
    static const byte TAG_Double = static_cast<byte>(6);
    static const byte TAG_Byte_Array = static_cast<byte>(7);
    static const byte TAG_String = static_cast<byte>(8);
    static const byte TAG_List = static_cast<byte>(9);
    static const byte TAG_Compound = static_cast<byte>(10);
	static const byte TAG_Int_Array = static_cast<byte>(11);

private:
    wstring name;

protected:
	Tag(const wstring &name);

public:
    virtual void write(DataOutput *dos) = 0;
    virtual void load(DataInput *dis)  = 0;
    virtual wstring toString() = 0;
    virtual byte getId() = 0;
    void print(ostream out);
    void print(char *prefix, wostream out);
    wstring getName();
    Tag *setName(const wstring& name);
    static Tag *readNamedTag(DataInput *dis);
    static void writeNamedTag(Tag *tag, DataOutput *dos);
    static Tag *newTag(byte type, const wstring &name);
    static const wchar_t *getTagName(byte type);
	virtual ~Tag() {}
	virtual bool equals(Tag *obj); // 4J Brought forward from 1.2
	virtual Tag *copy() = 0; // 4J Brought foward from 1.2
};
