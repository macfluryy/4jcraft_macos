#include "java/InputOutputStream/BufferedReader.h"

#include <algorithm>
#include <cassert>
#include <cstring>

#include "java/InputOutputStream/Reader.h"



BufferedReader::BufferedReader(Reader* in)
    : reader(in), readMark(0), bufferedMark(0), eofReached(false) {
    bufferSize = 64;
    buffer = new wchar_t[bufferSize];
    memset(buffer, 0, sizeof(wchar_t) * bufferSize);
    bufferMore();
}

BufferedReader::~BufferedReader() { delete[] buffer; }

void BufferedReader::bufferMore() {
    
    
    if (bufferedMark - readMark > (BUFFER_MORE_AMOUNT / 2)) return;

    if (bufferSize < (bufferedMark + BUFFER_MORE_AMOUNT)) {
        
        wchar_t* temp = new wchar_t[bufferSize * 2];
        memset(temp, 0, sizeof(wchar_t) * bufferSize * 2);
        std::copy(buffer, buffer + bufferSize, temp);

        delete[] buffer;
        buffer = temp;
        bufferSize = bufferSize * 2;
    }

    int value = 0;
    unsigned int newCharsBuffered = 0;
    while (newCharsBuffered < BUFFER_MORE_AMOUNT &&
           (value = reader->read()) != -1) {
        buffer[bufferedMark++] = value;
        newCharsBuffered++;
    }
}





void BufferedReader::close() { reader->close(); }





int BufferedReader::read() {
    
    assert(bufferedMark >= readMark);

    if (bufferedMark == readMark) {
        int value = reader->read();
        if (value == -1) return -1;

        buffer[bufferedMark++] = value;

        bufferMore();
    }

    return buffer[readMark++];
}































int BufferedReader::read(wchar_t cbuf[], unsigned int off, unsigned int len) {
    if (bufferSize < (bufferedMark + len)) {
        
        wchar_t* temp = new wchar_t[bufferSize * 2];
        memset(temp, 0, sizeof(wchar_t) * bufferSize * 2);
        std::copy(buffer, buffer + bufferSize, temp);

        delete[] buffer;
        buffer = temp;
        bufferSize = bufferSize * 2;
    }

    unsigned int charsRead = 0;
    while (charsRead < len && readMark <= bufferedMark) {
        cbuf[off + charsRead] = buffer[readMark++];
        charsRead++;
    }

    int value = 0;
    while (charsRead < len && (value = reader->read()) != -1) {
        buffer[bufferedMark++] = value;
        cbuf[off + charsRead] = value;
        charsRead++;
        readMark++;
    }

    bufferMore();

    return charsRead;
}






std::wstring BufferedReader::readLine() {
    std::wstring output = L"";
    bool newLineCharFound = false;

    while (readMark < bufferedMark) {
        wchar_t value = buffer[readMark++];

        if (!newLineCharFound) {
            if ((value == '\n') || (value == '\r')) {
                newLineCharFound = true;
            } else {
                output.push_back(value);
            }
        } else {
            if ((value != '\n') && (value != '\r')) {
                readMark--;  
                             
                break;
            }
        }

        
        
        bufferMore();
    }
    return output;
}