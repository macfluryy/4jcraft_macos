#pragma once

class Reader {
public:
    virtual ~Reader() {}

    virtual void close() = 0;  
                               
    virtual int read() = 0;    
    virtual int read(
        wchar_t cbuf[], unsigned int off,
        unsigned int len) = 0;  
};