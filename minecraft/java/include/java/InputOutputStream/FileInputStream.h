#pragma once
// 4J Stu - Represents Java standard library class

#include <cstdio>
#include <cstdint>

#include "../../../../Minecraft.World/ConsoleHelpers/ArrayWithLength.h" // 4jcraft TODO

#include "InputStream.h"

class File;

class FileInputStream : public InputStream {
public:
    FileInputStream(const File& file);
    virtual ~FileInputStream();
    virtual int read();
    virtual int read(byteArray b);
    virtual int read(byteArray b, unsigned int offset, unsigned int length);
    virtual void close();
    virtual int64_t skip(int64_t n);

private:
    std::FILE* m_fileHandle;
};
