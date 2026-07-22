#pragma once


#include <cstdint>
#include <vector>

#include "InputStream.h"

class ByteArrayInputStream : public InputStream {
protected:
    std::vector<uint8_t> buf;  
                               
    unsigned int count;  
                         
    unsigned int mark;   
    unsigned int pos;  
                       

public:
    ByteArrayInputStream(std::vector<uint8_t>& buf, unsigned int offset,
                         unsigned int length);
    ByteArrayInputStream(std::vector<uint8_t>& buf);
    
    ByteArrayInputStream(std::vector<uint8_t>&& buf);
    virtual ~ByteArrayInputStream();
    virtual int read();
    virtual int read(std::vector<uint8_t>& b);
    virtual int read(std::vector<uint8_t>& b, unsigned int offset,
                     unsigned int length);
    virtual void close();
    virtual int64_t skip(int64_t n);

    
    
    void reset() {
        buf = std::vector<uint8_t>();
        count = 0;
        mark = 0;
        pos = 0;
    }
};
