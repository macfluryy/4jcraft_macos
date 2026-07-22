#pragma once

#include <stdint.h>

#include <cstdint>
#include <vector>

#include "OutputStream.h"

class BufferedOutputStream : public OutputStream {
private:
    OutputStream* stream;

protected:
    std::vector<uint8_t> buf;  
    unsigned int count;        

public:
    BufferedOutputStream(OutputStream* out, int size);
    ~BufferedOutputStream();

    virtual void flush();
    virtual void close();
    virtual void write(const std::vector<uint8_t>& b, unsigned int offset,
                       unsigned int length);
    virtual void write(const std::vector<uint8_t>& b);
    virtual void write(unsigned int b);
};