#pragma once
// 4J Stu - Represents Java standard library class

#include <cstdio>

#include "../Streams/InputStream.h"

class File;

class FileInputStream : public InputStream {
public:
    FileInputStream(const File& file);
    virtual ~FileInputStream();
    virtual int read();
    virtual int read(byteArray b);
    virtual int read(byteArray b, unsigned int offset, unsigned int length);
    virtual void close();
    virtual __int64 skip(__int64 n);

private:
    std::FILE* m_fileHandle;
};
