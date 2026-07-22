#pragma once

#include <chrono>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include <bit>

#define MAKE_FOURCC(ch0, ch1, ch2, ch3)                                   \
    (static_cast<std::uint32_t>(static_cast<std::uint8_t>(ch0)) |         \
     (static_cast<std::uint32_t>(static_cast<std::uint8_t>(ch1)) << 8) |  \
     (static_cast<std::uint32_t>(static_cast<std::uint8_t>(ch2)) << 16) | \
     (static_cast<std::uint32_t>(static_cast<std::uint8_t>(ch3)) << 24))






#define SAVE_FILE_HEADER_SIZE 12

enum ESaveVersions {
    
    SAVE_FILE_VERSION_PRE_LAUNCH = 1,

    
    SAVE_FILE_VERSION_LAUNCH = 2,

    
    SAVE_FILE_VERSION_POST_LAUNCH = 3,

    
    
    SAVE_FILE_VERSION_NEW_END = 4,

    
    
    SAVE_FILE_VERSION_MOVED_STRONGHOLD = 5,

    
    SAVE_FILE_VERSION_CHANGE_MAP_DATA_MAPPING_SIZE = 6,

    
    SAVE_FILE_VERSION_DURANGO_CHANGE_MAP_DATA_MAPPING_SIZE = 7,

    
    
    SAVE_FILE_VERSION_COMPRESSED_CHUNK_STORAGE,

    
    SAVE_FILE_VERSION_CHUNK_INHABITED_TIME,

    
    

    SAVE_FILE_VERSION_NEXT,
};


#define SAVE_FILE_VERSION_DURANGO_CHANGE_MAP_DATA_MAPPING_SIZE 7

enum ESavePlatform {
    SAVE_FILE_PLATFORM_NONE = MAKE_FOURCC('N', 'O', 'N', 'E'),
    SAVE_FILE_PLATFORM_X360 = MAKE_FOURCC('X', '3', '6', '0'),
    SAVE_FILE_PLATFORM_XBONE = MAKE_FOURCC('X', 'B', '1', '_'),
    SAVE_FILE_PLATFORM_PS3 = MAKE_FOURCC('P', 'S', '3', '_'),
    SAVE_FILE_PLATFORM_PS4 = MAKE_FOURCC('P', 'S', '4', '_'),
    SAVE_FILE_PLATFORM_PSVITA = MAKE_FOURCC('P', 'S', 'V', '_'),
    SAVE_FILE_PLATFORM_WIN64 = MAKE_FOURCC('W', 'I', 'N', '_'),

#if defined(_WINDOWS64)
    SAVE_FILE_PLATFORM_LOCAL = SAVE_FILE_PLATFORM_WIN64
#else
    
    SAVE_FILE_PLATFORM_LOCAL = SAVE_FILE_PLATFORM_WIN64
#endif
};
#define SAVE_FILE_VERSION_NUMBER (SAVE_FILE_VERSION_NEXT - 1)

struct FileEntrySaveDataV1 {
public:
    wchar_t filename[64];  
    unsigned int length;   

    
    
    unsigned int startOffset;  
};



struct FileEntrySaveDataV2 {
public:
    wchar_t filename[64];  
    unsigned int length;   

    union {
        
        
        unsigned int startOffset;  
        
        
        
        
        unsigned int regionIndex;  
    };

    int64_t lastModifiedTime;  
};

typedef FileEntrySaveDataV2 FileEntrySaveData;

class FileEntry {
public:
    FileEntrySaveData data;

    unsigned int currentFilePointer;

    FileEntry() { memset(&data, 0, sizeof(FileEntrySaveData)); }

    FileEntry(wchar_t name[64], unsigned int length, unsigned int startOffset) {
        data.length = length;
        data.startOffset = startOffset;
        memset(&data.filename, 0, sizeof(wchar_t) * 64);
        memcpy(&data.filename, name, sizeof(wchar_t) * 64);

        data.lastModifiedTime = 0;

        currentFilePointer = data.startOffset;
    }

    unsigned int getFileSize() { return data.length; }
    bool isRegionFile() {
        return data.filename[0] == 0;
    }  
    unsigned int getRegionFileIndex() {
        return data.regionIndex;
    }  

    void updateLastModifiedTime() {
        auto now = std::chrono::system_clock::now().time_since_epoch();
        data.lastModifiedTime =
            std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
    }

    





    static bool newestFirst(FileEntry* a, FileEntry* b) {
        return a->data.lastModifiedTime > b->data.lastModifiedTime;
    }
};


class FileHeader {
    friend class ConsoleSaveFileOriginal;
    friend class ConsoleSaveFileSplit;

private:
    std::vector<FileEntry*> fileTable;
    ESavePlatform m_savePlatform;
    std::endian m_saveEndian;
    static const std::endian m_localEndian = std::endian::little;

    short m_saveVersion;
    short m_originalSaveVersion;

public:
    FileEntry* lastFile;

public:
    FileHeader();
    ~FileHeader();

protected:
    FileEntry* AddFile(const std::wstring& name, unsigned int length = 0);
    void RemoveFile(FileEntry*);
    void WriteHeader(void* saveMem);
    void ReadHeader(void* saveMem,
                    ESavePlatform plat = SAVE_FILE_PLATFORM_LOCAL);

    unsigned int GetStartOfNextData();

    unsigned int GetFileSize();

    void AdjustStartOffsets(FileEntry* file, unsigned int nNumberOfBytesToWrite,
                            bool subtract = false);

    bool fileExists(const std::wstring& name);

    std::vector<FileEntry*>* getFilesWithPrefix(const std::wstring& prefix);

    std::vector<FileEntry*>* getValidPlayerDatFiles();

    void setSaveVersion(int version) { m_saveVersion = version; }
    int getSaveVersion() { return m_saveVersion; }
    void setOriginalSaveVersion(int version) {
        m_originalSaveVersion = version;
    }
    int getOriginalSaveVersion() { return m_originalSaveVersion; }
    ESavePlatform getSavePlatform() { return m_savePlatform; }
    void setPlatform(ESavePlatform plat) { m_savePlatform = plat; }
    bool isSaveEndianDifferent() { return m_saveEndian != m_localEndian; }
    void setLocalPlatform() {
        m_savePlatform = SAVE_FILE_PLATFORM_LOCAL;
        m_saveEndian = m_localEndian;
    }
    std::endian getSaveEndian() { return m_saveEndian; }
    static std::endian getLocalEndian() { return m_localEndian; }
    void setEndian(std::endian endian) { m_saveEndian = endian; }
    static std::endian getEndian(ESavePlatform plat);
    bool isLocalEndianDifferent(ESavePlatform plat) {
        return m_localEndian != getEndian(plat);
    }
};

#undef MAKE_FOURCC