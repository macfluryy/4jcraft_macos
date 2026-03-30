#pragma once

#include "../../../../Minecraft.World/ConsoleHelpers/ArrayWithLength.h" // 4jcraft TODO

class DataOutput {
public:
    virtual void write(unsigned int b) = 0;
    virtual void write(byteArray b) = 0;
    virtual void write(byteArray b, unsigned int offset,
                       unsigned int length) = 0;
    virtual void writeByte(uint8_t a) = 0;
    virtual void writeDouble(double a) = 0;
    virtual void writeFloat(float a) = 0;
    virtual void writeInt(int a) = 0;
    virtual void writeLong(int64_t a) = 0;
    virtual void writeShort(short a) = 0;
    virtual void writeBoolean(bool v) = 0;
    virtual void writeChar(wchar_t v) = 0;
    virtual void writeChars(const std::wstring& s) = 0;
    virtual void writeUTF(const std::wstring& a) = 0;
    virtual void writePlayerUID(unsigned long long player) = 0;  // 4J Added
};