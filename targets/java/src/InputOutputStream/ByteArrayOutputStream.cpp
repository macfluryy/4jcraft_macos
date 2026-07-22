#include "java/InputOutputStream/ByteArrayOutputStream.h"

#include <assert.h>

#include <algorithm>
#include <cstring>



ByteArrayOutputStream::ByteArrayOutputStream() {
    count = 0;
    buf = std::vector<uint8_t>(32);
}



ByteArrayOutputStream::ByteArrayOutputStream(unsigned int size) {
    count = 0;
    buf = std::vector<uint8_t>(size);
}

ByteArrayOutputStream::~ByteArrayOutputStream() {}




void ByteArrayOutputStream::write(unsigned int b) {
    
    if (count + 1 >= buf.size()) buf.resize(buf.size() * 2);

    buf[count] = (uint8_t)b;
    count++;
}




void ByteArrayOutputStream::write(const std::vector<uint8_t>& b) {
    write(b, 0, b.size());
}




void ByteArrayOutputStream::write(const std::vector<uint8_t>& b,
                                  unsigned int offset, unsigned int length) {
    assert(b.size() >= offset + length);

    
    if (count + length >= buf.size())
        buf.resize(
            std::max(count + length + 1, (unsigned int)(buf.size() * 2)));

    std::memcpy(&buf[count], &b[offset], length);
    
    

    count += length;
}




void ByteArrayOutputStream::close() {}





std::vector<uint8_t> ByteArrayOutputStream::toByteArray() {
    std::vector<uint8_t> out(count);
    memcpy(out.data(), buf.data(), count);
    return out;
}