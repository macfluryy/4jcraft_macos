#pragma once
// 4J Stu - Represents Java standard library class (although we miss out an
// intermediate inheritance class that we don't care about)

#include <string>

#include "InputStream.h"
#include "DataInput.h"

#include "../../../../Minecraft.World/ConsoleHelpers/ArrayWithLength.h" // 4jcraft TODO

class DataInputStream : public InputStream, public DataInput {
private:
    InputStream* stream;

public:
    DataInputStream(InputStream* in);
    virtual int read();
    virtual int read(byteArray b);
    virtual int read(byteArray b, unsigned int offset, unsigned int length);
    virtual void close();
    virtual bool readBoolean();
    virtual uint8_t readByte();
    virtual unsigned char readUnsignedByte();
    virtual wchar_t readChar();
    virtual bool readFully(byteArray b);
    virtual bool readFully(charArray b);
    virtual double readDouble();
    virtual float readFloat();
    virtual int readInt();
    virtual int64_t readLong();
    virtual short readShort();
    virtual unsigned short readUnsignedShort();
    virtual std::wstring readUTF();
    void deleteChildStream();
    virtual int readUTFChar();
    virtual unsigned long long readPlayerUID();  // 4J Added
    virtual int64_t skip(int64_t n);
    virtual int skipBytes(int n);
};