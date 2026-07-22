#include "java/InputOutputStream/DataOutputStream.h"

#include <stdio.h>

#include <bit>
#include <string>
#include <vector>

#include "java/InputOutputStream/OutputStream.h"




DataOutputStream::DataOutputStream(OutputStream* out)
    : stream(out), written(0) {}




void DataOutputStream::deleteChildStream() { delete stream; }





void DataOutputStream::write(unsigned int b) {
    if (stream == nullptr) {
        fprintf(stderr,
                "DataOutputStream::write(unsigned int) called but underlying "
                "stream is nullptr\n");
        return;
    }
    stream->write(b);
    
    written++;
}

void DataOutputStream::flush() {
    if (stream == nullptr) {
        fprintf(stderr,
                "DataOutputStream::flush() called but underlying stream is "
                "nullptr\n");
        return;
    }
    stream->flush();
}




void DataOutputStream::write(const std::vector<uint8_t>& b) {
    write(b, 0, b.size());
}





void DataOutputStream::write(const std::vector<uint8_t>& b, unsigned int offset,
                             unsigned int length) {
    if (stream == nullptr) {
        fprintf(stderr,
                "DataOutputStream::write(std::vector<uint8_t>,...) called but "
                "underlying "
                "stream is nullptr\n");
        return;
    }
    stream->write(b, offset, length);
    
    written += length;
}




void DataOutputStream::close() {
    if (stream == nullptr) {
        fprintf(stderr,
                "DataOutputStream::close() called but underlying stream is "
                "nullptr\n");
        return;
    }
    stream->close();
}




void DataOutputStream::writeByte(uint8_t a) {
    stream->write(static_cast<unsigned int>(a));
}






void DataOutputStream::writeDouble(double a) {
    int64_t bits = std::bit_cast<int64_t>(a);

    writeLong(bits);
    
    written += 8;
}





void DataOutputStream::writeFloat(float a) {
    int bits = std::bit_cast<int>(a);

    writeInt(bits);
    
    written += 4;
}





void DataOutputStream::writeInt(int a) {
    stream->write((a >> 24) & 0xff);
    stream->write((a >> 16) & 0xff);
    stream->write((a >> 8) & 0xff);
    stream->write(a & 0xff);
    
    written += 4;
}





void DataOutputStream::writeLong(int64_t a) {
    stream->write((a >> 56) & 0xff);
    stream->write((a >> 48) & 0xff);
    stream->write((a >> 40) & 0xff);
    stream->write((a >> 32) & 0xff);
    stream->write((a >> 24) & 0xff);
    stream->write((a >> 16) & 0xff);
    stream->write((a >> 8) & 0xff);
    stream->write(a & 0xff);
    
    written += 4;
}





void DataOutputStream::writeShort(short a) {
    stream->write((a >> 8) & 0xff);
    stream->write(a & 0xff);
    
    written += 2;
}

void DataOutputStream::writeUnsignedShort(unsigned short a) {
    if (stream == nullptr) {
        fprintf(
            stderr,
            "DataOutputStream::writeUnsignedShort() but underlying stream is "
            "nullptr\n");
        return;
    }
    stream->write(static_cast<unsigned int>((a >> 8) & 0xff));
    stream->write(static_cast<unsigned int>(a & 0xff));
    written += 2;
}





void DataOutputStream::writeChar(wchar_t v) {
    stream->write((v >> 8) & 0xff);
    stream->write(v & 0xff);
    
    written += 2;
}





void DataOutputStream::writeChars(const std::wstring& str) {
    for (unsigned int i = 0; i < str.length(); i++) {
        writeChar(str.at(i));
        
    }
    
}





void DataOutputStream::writeBoolean(bool b) {
    stream->write(b ? 1 : 0);
    
    written += 1;
}











void DataOutputStream::writeUTF(const std::wstring& str) {
    int strlen = (int)str.length();
    int utflen = 0;
    int c, count = 0;

    
    for (int i = 0; i < strlen; i++) {
        c = str.at(i);
        if ((c >= 0x0001) && (c <= 0x007F)) {
            utflen++;
        } else if (c > 0x07FF) {
            utflen += 3;
        } else {
            utflen += 2;
        }
    }

    
    
    

    std::vector<uint8_t> bytearr(utflen + 2);

    bytearr[count++] = (uint8_t)((utflen >> 8) & 0xFF);
    bytearr[count++] = (uint8_t)((utflen >> 0) & 0xFF);

    int i = 0;
    for (i = 0; i < strlen; i++) {
        c = str.at(i);
        if (!((c >= 0x0001) && (c <= 0x007F))) break;
        bytearr[count++] = (uint8_t)c;
    }

    for (; i < strlen; i++) {
        c = str.at(i);
        if ((c >= 0x0001) && (c <= 0x007F)) {
            bytearr[count++] = (uint8_t)c;

        } else if (c > 0x07FF) {
            bytearr[count++] = (uint8_t)(0xE0 | ((c >> 12) & 0x0F));
            bytearr[count++] = (uint8_t)(0x80 | ((c >> 6) & 0x3F));
            bytearr[count++] = (uint8_t)(0x80 | ((c >> 0) & 0x3F));
        } else {
            bytearr[count++] = (uint8_t)(0xC0 | ((c >> 6) & 0x1F));
            bytearr[count++] = (uint8_t)(0x80 | ((c >> 0) & 0x3F));
        }
    }
    write(bytearr, 0, utflen + 2);
}


void DataOutputStream::writePlayerUID(unsigned long long player) {
    writeLong(player);
}
