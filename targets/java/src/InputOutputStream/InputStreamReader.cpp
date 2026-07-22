#include "java/InputOutputStream/InputStreamReader.h"

#include "java/InputOutputStream/DataInputStream.h"

class InputStream;




InputStreamReader::InputStreamReader(InputStream* in)
    : stream(new DataInputStream(in)) {}





void InputStreamReader::close() { stream->close(); }




int InputStreamReader::read() { return stream->readUTFChar(); }









int InputStreamReader::read(wchar_t cbuf[], unsigned int offset,
                            unsigned int length) {
    unsigned int charsRead = 0;
    for (unsigned int i = offset; i < offset + length; i++) {
        wchar_t value = (wchar_t)stream->readUTFChar();
        if (value != -1) {
            cbuf[i] = value;
            charsRead++;
        }
        
        
        else
            break;
    }
    return charsRead;
}