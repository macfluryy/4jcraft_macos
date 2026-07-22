#include "java/InputOutputStream/BufferedOutputStream.h"

#include <stdio.h>

#include "java/InputOutputStream/OutputStream.h"




BufferedOutputStream::BufferedOutputStream(OutputStream* out, int size) {
    stream = out;
    buf = std::vector<uint8_t>(size);
    count = 0;
}

BufferedOutputStream::~BufferedOutputStream() {
    
    
    
    
}



void BufferedOutputStream::flush() {
    if (stream == nullptr) {
        fprintf(stderr,
                "BufferedOutputStream::flush() called but underlying stream is "
                "nullptr\n");
        return;
    }

    if (count > 0) {
        stream->write(buf, 0, count);
        count = 0;
    }
}




void BufferedOutputStream::close() {
    flush();
    if (stream == nullptr) {
        fprintf(stderr,
                "BufferedOutputStream::close() called but underlying stream is "
                "nullptr\n");
        return;
    }
    stream->close();
}















void BufferedOutputStream::write(const std::vector<uint8_t>& b,
                                 unsigned int offset, unsigned int length) {
    
    
    if (length >= buf.size()) {
        flush();
        stream->write(b, offset, length);
    } else {
        for (unsigned int i = 0; i < length; i++) {
            write(static_cast<unsigned int>(b[offset + i]));
        }
    }
}







void BufferedOutputStream::write(const std::vector<uint8_t>& b) {
    write(b, 0, b.size());
}






void BufferedOutputStream::write(unsigned int b) {
    buf[count++] = (uint8_t)b;
    if (count == buf.size()) {
        flush();
    }
}
