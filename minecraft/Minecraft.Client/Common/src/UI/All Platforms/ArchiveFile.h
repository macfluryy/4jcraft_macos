#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <format>
#include <string>

#include "java/File.h"

class DataInputStream;

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
    std::vector<uint8_t> getFile(const std::wstring& filename);
};
