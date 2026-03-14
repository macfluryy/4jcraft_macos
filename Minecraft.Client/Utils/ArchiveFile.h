#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

#include "../../Minecraft.World/IO/Files/File.h"
#include "../../Minecraft.World/Util/ArrayWithLength.h"

class ArchiveFile {
protected:
    File m_sourcefile;
    std::uint8_t* m_cachedData;

    typedef struct _MetaData {
        std::wstring filename;
        int ptr;
        int filesize;
        bool isCompressed;

    } MetaData, *PMetaData;

    std::unordered_map<std::wstring, PMetaData> m_index;

public:
    void _readHeader(DataInputStream* dis);

    ArchiveFile(File file);
    ~ArchiveFile();

    std::vector<std::wstring>* getFileList();
    bool hasFile(const std::wstring& filename);
    int getFileSize(const std::wstring& filename);
    byteArray getFile(const std::wstring& filename);
};
