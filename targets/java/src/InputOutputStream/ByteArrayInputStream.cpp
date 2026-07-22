






#include "java/InputOutputStream/ByteArrayInputStream.h"

#include <algorithm>
#include <cstring>
#include <vector>

ByteArrayInputStream::ByteArrayInputStream(std::vector<uint8_t>& buf,
                                           unsigned int offset,
                                           unsigned int length)
    : pos(offset),
      count(std::min(offset + length, (unsigned int)buf.size())),
      mark(offset) {
    this->buf = buf;
}




ByteArrayInputStream::ByteArrayInputStream(std::vector<uint8_t>& buf)
    : pos(0), count(buf.size()), mark(0) {
    this->buf = buf;
}


ByteArrayInputStream::ByteArrayInputStream(std::vector<uint8_t>&& buf)
    : buf(std::move(buf)), pos(0), count(this->buf.size()), mark(0) {
}






int ByteArrayInputStream::read() {
    if (pos >= count)
        return -1;
    else
        return static_cast<unsigned int>(buf[pos++]);
}






















int ByteArrayInputStream::read(std::vector<uint8_t>& b) {
    return read(b, 0, b.size());
}














int ByteArrayInputStream::read(std::vector<uint8_t>& b, unsigned int offset,
                               unsigned int length) {
    if (pos == count) return -1;

    int k = std::min(length, count - pos);
    std::memcpy(&b[offset], &buf[pos], k);
    
    

    pos += k;

    return k;
}




void ByteArrayInputStream::close() { return; }







int64_t ByteArrayInputStream::skip(int64_t n) {
    int newPos = pos + n;

    if (newPos > count) newPos = count;

    int k = newPos - pos;
    pos = newPos;

    return k;
}

ByteArrayInputStream::~ByteArrayInputStream() {}
