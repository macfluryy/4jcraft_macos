#include "java/InputOutputStream/DataInputStream.h"

#include <stdio.h>

#include <bit>
#include <cstdint>
#include <string>
#include <vector>

#include "java/InputOutputStream/InputStream.h"




DataInputStream::DataInputStream(InputStream* in) : stream(in) {}







int DataInputStream::read() {
    if (stream == nullptr) {
        fprintf(stderr,
                "DataInputStream::read() called but underlying stream is "
                "nullptr\n");
        return -1;
    }
    return stream->read();
}


























int DataInputStream::read(std::vector<uint8_t>& b) {
    if (stream == nullptr) {
        fprintf(stderr,
                "DataInputStream::read(std::vector<uint8_t>) called but "
                "underlying stream is "
                "nullptr\n");
        return -1;
    }
    return read(b, 0, b.size());
}






























int DataInputStream::read(std::vector<uint8_t>& b, unsigned int offset,
                          unsigned int length) {
    if (stream == nullptr) {
        fprintf(stderr,
                "DataInputStream::read(std::vector<uint8_t>,offset,length) "
                "called but "
                "underlying stream is nullptr\n");
        return -1;
    }
    return stream->read(b, offset, length);
}



void DataInputStream::close() {
    if (stream == nullptr) {
        fprintf(stderr,
                "DataInputStream::close() called but underlying stream is "
                "nullptr\n");
        return;
    }
    stream->close();
}




bool DataInputStream::readBoolean() {
    if (stream == nullptr) {
        fprintf(stderr,
                "DataInputStream::readBoolean() but underlying stream is "
                "nullptr\n");
        return false;
    }
    return stream->read() != 0;
}





uint8_t DataInputStream::readByte() {
    if (stream == nullptr) {
        fprintf(
            stderr,
            "DataInputStream::readByte() but underlying stream is nullptr\n");
        return 0;
    }
    return (uint8_t)stream->read();
}

unsigned char DataInputStream::readUnsignedByte() {
    if (stream == nullptr) {
        fprintf(stderr,
                "DataInputStream::readUnsignedByte() but underlying stream is "
                "nullptr\n");
        return 0;
    }
    return (unsigned char)stream->read();
}







wchar_t DataInputStream::readChar() {
    if (stream == nullptr) {
        fprintf(
            stderr,
            "DataInputStream::readChar() but underlying stream is nullptr\n");
        return 0;
    }
    int a = stream->read();
    int b = stream->read();
    return (wchar_t)((a << 8) | (b & 0xff));
}
















bool DataInputStream::readFully(std::vector<uint8_t>& b) {
    
    
    
    
    if (stream == nullptr) {
        fprintf(stderr,
                "DataInputStream::readFully(std::vector<uint8_t>) but "
                "underlying stream is "
                "nullptr\n");
        return false;
    }
    for (unsigned int i = 0; i < b.size(); i++) {
        int byteRead = stream->read();
        if (byteRead == -1) {
            return false;
        } else {
            b[i] = static_cast<uint8_t>(byteRead);
        }
    }
    return true;
}

bool DataInputStream::readFully(std::vector<char>& b) {
    
    
    
    
    if (stream == nullptr) {
        fprintf(stderr,
                "DataInputStream::readFully(std::vector<char>) but underlying "
                "stream is "
                "nullptr\n");
        return false;
    }
    for (unsigned int i = 0; i < b.size(); i++) {
        int byteRead = stream->read();
        if (byteRead == -1) {
            return false;
        } else {
            b[i] = byteRead;
        }
    }
    return true;
}







double DataInputStream::readDouble() {
    int64_t bits = readLong();

    return std::bit_cast<double>(bits);
}






float DataInputStream::readFloat() {
    int bits = readInt();

    return std::bit_cast<float>(bits);
}









int DataInputStream::readInt() {
    if (stream == nullptr) {
        fprintf(
            stderr,
            "DataInputStream::readInt() but underlying stream is nullptr\n");
        return 0;
    }
    int a = stream->read();
    int b = stream->read();
    int c = stream->read();
    int d = stream->read();
    int bits = (((a & 0xff) << 24) | ((b & 0xff) << 16) | ((c & 0xff) << 8) |
                (d & 0xff));
    return bits;
}


















int64_t DataInputStream::readLong() {
    if (stream == nullptr) {
        fprintf(
            stderr,
            "DataInputStream::readLong() but underlying stream is nullptr\n");
        return 0;
    }
    int64_t a = stream->read();
    int64_t b = stream->read();
    int64_t c = stream->read();
    int64_t d = stream->read();
    int64_t e = stream->read();
    int64_t f = stream->read();
    int64_t g = stream->read();
    int64_t h = stream->read();

    int64_t bits =
        (((a & 0xff) << 56) | ((b & 0xff) << 48) | ((c & 0xff) << 40) |
         ((d & 0xff) << 32) | ((e & 0xff) << 24) | ((f & 0xff) << 16) |
         ((g & 0xff) << 8) | ((h & 0xff)));

    return bits;
}







short DataInputStream::readShort() {
    if (stream == nullptr) {
        fprintf(
            stderr,
            "DataInputStream::readShort() but underlying stream is nullptr\n");
        return 0;
    }
    int a = stream->read();
    int b = stream->read();
    return (short)((a << 8) | (b & 0xff));
}

unsigned short DataInputStream::readUnsignedShort() {
    if (stream == nullptr) {
        fprintf(stderr,
                "DataInputStream::readUnsignedShort() but underlying stream is "
                "nullptr\n");
        return 0;
    }
    int a = stream->read();
    int b = stream->read();
    return static_cast<unsigned short>(((a & 0xff) << 8) | (b & 0xff));
}















































std::wstring DataInputStream::readUTF() {
    std::wstring outputString;
    if (stream == nullptr) {
        fprintf(
            stderr,
            "DataInputStream::readUTF() but underlying stream is nullptr\n");
        return outputString;
    }
    int a = stream->read();
    int b = stream->read();
    unsigned short UTFLength = (unsigned short)(((a & 0xff) << 8) | (b & 0xff));

    
    
    

    





    unsigned short currentByteIndex = 0;
    while (currentByteIndex < UTFLength) {
        int firstByte = stream->read();
        currentByteIndex++;

        if (firstByte == -1)
            
            break;

        
        
        
        
        

        
        
        
        
        
        
        if (((firstByte & 0xC0) == 0x80) || ((firstByte & 0xF0) == 0xF0)) {
            
            break;
        } else if ((firstByte & 0x80) == 0x00) {
            
            wchar_t readChar = (wchar_t)firstByte;
            outputString.push_back(readChar);
            continue;
        } else if ((firstByte & 0xE0) == 0xC0) {
            

            
            if (!(currentByteIndex < UTFLength)) {
                
                break;
            }

            int secondByte = stream->read();
            currentByteIndex++;

            
            if (secondByte == -1) {
                
                break;
            }
            
            else if ((secondByte & 0xC0) != 0x80) {
                
                break;
            }

            wchar_t readChar =
                (wchar_t)(((firstByte & 0x1F) << 6) | (secondByte & 0x3F));
            outputString.push_back(readChar);
            continue;
        } else if ((firstByte & 0xF0) == 0xE0) {
            

            
            if (!(currentByteIndex < UTFLength)) {
                
                break;
            }

            int secondByte = stream->read();
            currentByteIndex++;

            
            if (secondByte == -1) {
                
                break;
            }

            
            if (!(currentByteIndex < UTFLength)) {
                
                break;
            }

            int thirdByte = stream->read();
            currentByteIndex++;

            
            if (thirdByte == -1) {
                
                break;
            }
            
            else if (((secondByte & 0xC0) != 0x80) ||
                     ((thirdByte & 0xC0) != 0x80)) {
                
                break;
            }

            wchar_t readChar =
                (wchar_t)(((firstByte & 0x0F) << 12) |
                          ((secondByte & 0x3F) << 6) | (thirdByte & 0x3F));
            outputString.push_back(readChar);
            continue;
        }
    }

    return outputString;
}

int DataInputStream::readUTFChar() {
    int returnValue = -1;
    if (stream == nullptr) {
        fprintf(stderr,
                "DataInputStream::readUTFChar() but underlying stream is "
                "nullptr\n");
        return returnValue;
    }
    int firstByte = stream->read();

    if (firstByte == -1)
        
        return returnValue;

    
    
    
    
    

    
    
    
    
    
    
    if (((firstByte & 0xC0) == 0x80) || ((firstByte & 0xF0) == 0xF0)) {
        
        return returnValue;
    } else if ((firstByte & 0x80) == 0x00) {
        
        returnValue = firstByte;
    } else if ((firstByte & 0xE0) == 0xC0) {
        
        int secondByte = stream->read();

        
        if (secondByte == -1) {
            
            return returnValue;
        }
        
        else if ((secondByte & 0xC0) != 0x80) {
            
            return returnValue;
        }

        returnValue = ((firstByte & 0x1F) << 6) | (secondByte & 0x3F);
    } else if ((firstByte & 0xF0) == 0xE0) {
        

        int secondByte = stream->read();

        
        if (secondByte == -1) {
            
            return returnValue;
        }

        int thirdByte = stream->read();

        
        if (thirdByte == -1) {
            
            return returnValue;
        }
        
        else if (((secondByte & 0xC0) != 0x80) ||
                 ((thirdByte & 0xC0) != 0x80)) {
            
            return returnValue;
        }

        returnValue = (((firstByte & 0x0F) << 12) | ((secondByte & 0x3F) << 6) |
                       (thirdByte & 0x3F));
    }
    return returnValue;
}


unsigned long long DataInputStream::readPlayerUID() {
    unsigned long long returnValue;
    returnValue = readLong();
    return returnValue;
}

void DataInputStream::deleteChildStream() { delete stream; }







int64_t DataInputStream::skip(int64_t n) { return stream->skip(n); }

int DataInputStream::skipBytes(int n) { return skip(n); }
