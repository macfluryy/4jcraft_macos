#pragma once
// 4J Stu - Represents Java standard lib abstract

#include <cstdio>

#include "../../../../Minecraft.World/ConsoleHelpers/ArrayWithLength.h" // 4jcraft TODO

#include "OutputStream.h"

class File;

class FileOutputStream : public OutputStream {
public:
    FileOutputStream(const File& file);
    virtual ~FileOutputStream();
    virtual void write(unsigned int b);
    virtual void write(byteArray b);
    virtual void write(byteArray b, unsigned int offset, unsigned int length);
    virtual void close();
    virtual void flush();

private:
    std::FILE* m_fileHandle;
};
