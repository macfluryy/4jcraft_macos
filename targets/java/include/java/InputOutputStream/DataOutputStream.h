#pragma once



#include <stdint.h>

#include <string>

#include "DataOutput.h"
#include "OutputStream.h"

class DataOutputStream : public OutputStream, public DataOutput {
private:
    OutputStream* stream;

protected:
    int written;  
                  

public:
    DataOutputStream(OutputStream* out);

    
    void deleteChildStream();

    virtual void write(unsigned int b);
    virtual void write(const std::vector<uint8_t>& b);
    virtual void write(const std::vector<uint8_t>& b, unsigned int offset,
                       unsigned int length);
    virtual void close();
    virtual void writeByte(uint8_t a);
    virtual void writeDouble(double a);
    virtual void writeFloat(float a);
    virtual void writeInt(int a);
    virtual void writeLong(int64_t a);
    virtual void writeShort(short a);
    virtual void writeUnsignedShort(unsigned short a);
    virtual void writeChar(wchar_t a);
    virtual void writeChars(const std::wstring& a);
    virtual void writeBoolean(bool b);
    virtual void writeUTF(const std::wstring& a);
    virtual void writePlayerUID(unsigned long long player);
    virtual void flush();
};