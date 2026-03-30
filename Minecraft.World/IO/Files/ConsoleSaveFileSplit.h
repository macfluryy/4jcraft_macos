#pragma once
#include <mutex>

#include "FileHeader.h"
#include "ConsoleSavePath.h"
#include "ConsoleSaveFile.h"

class ProgressRenderer;

class ConsoleSaveFileSplit : public ConsoleSaveFile {
private:
    FileHeader header;

    static const int WRITE_BANDWIDTH_BYTESPERSECOND =
        1048576;  // Average bytes per second we will cap to when writing region
                  // files during the tick() method
    static const int WRITE_BANDWIDTH_MEASUREMENT_PERIOD_SECONDS =
        10;  // Time period over which the bytes per second average is
             // calculated
    static const int WRITE_TICK_RATE_MS =
        500;  // Time between attempts to work out which regions we should write
              // during the tick
    static const int WRITE_MAX_WRITE_PER_TICK =
        WRITE_BANDWIDTH_BYTESPERSECOND;  // Maximum number of bytes we can add
                                         // in a single tick

    class WriteHistory {
    public:
        std::int64_t writeTime;
        unsigned int writeSize;
    };

    class DirtyRegionFile {
    public:
        std::int64_t lastWritten;
        unsigned int fileRef;
        bool operator<(const DirtyRegionFile& rhs) const {
            return lastWritten < rhs.lastWritten;
        }
    };

    class RegionFileReference {
    public:
        RegionFileReference(int index, unsigned int regionIndex,
                            unsigned int length = 0,
                            unsigned char* data = nullptr);
        ~RegionFileReference();
        void Compress();    // Compress from data to dataCompressed
        void Decompress();  // Decompress from dataCompressed -> data
        unsigned int GetCompressedSize();  // Gets byte size for what this
                                           // region will compress to
        void ReleaseCompressed();          // Release dataCompressed
        FileEntry* fileEntry;
        unsigned char* data;
        unsigned char* dataCompressed;
        unsigned int dataCompressedSize;
        int index;
        bool dirty;
        std::int64_t lastWritten;
    };
    std::unordered_map<unsigned int, RegionFileReference*> regionFiles;
    std::vector<WriteHistory> writeHistory;
    std::int64_t m_lastTickTime;

    FileEntry* GetRegionFileEntry(unsigned int regionIndex);

    std::wstring m_fileName;
    bool m_autosave;

    //	void* hHeap;
    static void* pvHeap;
    static unsigned int pagesCommitted;
#if defined(_LARGE_WORLDS)
    static const unsigned int CSF_PAGE_SIZE = 64 * 1024;
    static const unsigned int MAX_PAGE_COUNT =
        32 * 1024;  // 2GB virtual allocation
#else
    static const unsigned int CSF_PAGE_SIZE = 64 * 1024;
    static const unsigned int MAX_PAGE_COUNT = 1024;
#endif
    void* pvSaveMem;

    std::recursive_mutex m_lock;

    void PrepareForWrite(FileEntry* file, unsigned int nNumberOfBytesToWrite);
    void MoveDataBeyond(FileEntry* file, unsigned int nNumberOfBytesToWrite);

    bool GetNumericIdentifierFromName(const std::wstring& fileName,
                                      unsigned int* idOut);
    std::wstring GetNameFromNumericIdentifier(unsigned int idIn);
    void processSubfilesForWrite();
    void processSubfilesAfterWrite();

public:
    static int SaveSaveDataCallback(void* lpParam, bool bRes);
    static int SaveRegionFilesCallback(void* lpParam, bool bRes);

private:
    void _init(const std::wstring& fileName, void* pvSaveData,
               unsigned int fileSize, ESavePlatform plat);

public:
    ConsoleSaveFileSplit(const std::wstring& fileName,
                         void* pvSaveData = nullptr, unsigned int fileSize = 0,
                         bool forceCleanSave = false,
                         ESavePlatform plat = SAVE_FILE_PLATFORM_LOCAL);
    ConsoleSaveFileSplit(ConsoleSaveFile* sourceSave,
                         bool alreadySmallRegions = true,
                         ProgressListener* progress = nullptr);
    virtual ~ConsoleSaveFileSplit();

    // 4J Stu - Initial implementation is intended to have a similar interface
    // to the standard Xbox file access functions

    virtual FileEntry* createFile(const ConsoleSavePath& fileName);
    virtual void deleteFile(FileEntry* file);

    virtual void setFilePointer(FileEntry* file, unsigned int distanceToMove,
                                SaveFileSeekOrigin seekOrigin);
    virtual bool writeFile(FileEntry* file, const void* lpBuffer,
                           unsigned int nNumberOfBytesToWrite,
                           unsigned int* lpNumberOfBytesWritten);
    virtual bool zeroFile(FileEntry* file, unsigned int nNumberOfBytesToWrite,
                          unsigned int* lpNumberOfBytesWritten);
    virtual bool readFile(FileEntry* file, void* lpBuffer,
                          unsigned int nNumberOfBytesToRead,
                          unsigned int* lpNumberOfBytesRead);
    virtual bool closeHandle(FileEntry* file);

    virtual void finalizeWrite();
    virtual void tick();

    virtual bool doesFileExist(ConsoleSavePath file);

    virtual void Flush(bool autosave, bool updateThumbnail = true);

#if !defined(_CONTENT_PACKAGE)
    virtual void DebugFlushToFile(void* compressedData = nullptr,
                                  unsigned int compressedDataSize = 0);
#endif
    virtual unsigned int getSizeOnDisk();

    virtual std::wstring getFilename();

    virtual std::vector<FileEntry*>* getFilesWithPrefix(
        const std::wstring& prefix);
    virtual std::vector<FileEntry*>* getRegionFilesByDimension(
        unsigned int dimensionIndex);

    virtual int getSaveVersion();
    virtual int getOriginalSaveVersion();

    virtual void LockSaveAccess();
    virtual void ReleaseSaveAccess();

    virtual ESavePlatform getSavePlatform();
    virtual bool isSaveEndianDifferent();
    virtual void setLocalPlatform();
    virtual void setPlatform(ESavePlatform plat);
    virtual ByteOrder getSaveEndian();
    virtual ByteOrder getLocalEndian();
    virtual void setEndian(ByteOrder endian);

    virtual void ConvertRegionFile(File sourceFile);
    virtual void ConvertToLocalPlatform();
};
