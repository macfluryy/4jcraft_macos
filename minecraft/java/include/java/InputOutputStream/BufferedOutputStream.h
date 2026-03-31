#pragma once

#include "OutputStream.h"
#include "../../../../Minecraft.World/ConsoleHelpers/ArrayWithLength.h" // 4jcraft TODO

class BufferedOutputStream : public OutputStream {
private:
    OutputStream* stream;

protected:
    std::vector<uint8_t> buf;       // The internal buffer where data is stored.
    unsigned int count;  // The number of valid bytes in the buffer.

public:
    BufferedOutputStream(OutputStream* out, int size);
    ~BufferedOutputStream();

    virtual void flush();
    virtual void close();
    virtual void write(const std::vector<uint8_t>& b, unsigned int offset, unsigned int length);
    virtual void write(const std::vector<uint8_t>& b);
    virtual void write(unsigned int b);
};