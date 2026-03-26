#pragma once

#include "FileHeader.h"
#include "ConsoleSavePath.h"
#include "ConsoleSaveFile.h"

class ConsoleSaveFileOriginal : public ConsoleSaveFile {
private:
    FileHeader header;

    std::wstring m_fileName;

    //	HANDLE hHeap;
    static void* pvHeap;
    static unsigned int pagesCommitted;
#ifdef _LARGE_WORLDS
    static const unsigned int CSF_PAGE_SIZE = 64 * 1024;
    static const unsigned int MAX_PAGE_COUNT =
        32 * 1024;  // 2GB virtual allocation
#elif 0
    static const unsigned int CSF_PAGE_SIZE = 1024 * 1024;
    static const unsigned int MAX_PAGE_COUNT = 64;
#else
    static const unsigned int CSF_PAGE_SIZE = 64 * 1024;
    static const unsigned int MAX_PAGE_COUNT = 1024;
#endif
    void* pvSaveMem;

    CRITICAL_SECTION m_lock;

    void PrepareForWrite(FileEntry* file, unsigned int nNumberOfBytesToWrite);
    void MoveDataBeyond(FileEntry* file, unsigned int nNumberOfBytesToWrite);

public:
#if (0 || 0 || 0 || \
     0 || defined _WINDOWS64)
    static int SaveSaveDataCallback(void* lpParam, bool bRes);
#endif
    ConsoleSaveFileOriginal(const std::wstring& fileName,
                            void* pvSaveData = NULL, unsigned int fileSize = 0,
                            bool forceCleanSave = false,
                            ESavePlatform plat = SAVE_FILE_PLATFORM_LOCAL);
    virtual ~ConsoleSaveFileOriginal();

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

    virtual bool doesFileExist(ConsoleSavePath file);

    virtual void Flush(bool autosave, bool updateThumbnail = true);

#ifndef _CONTENT_PACKAGE
    virtual void DebugFlushToFile(void* compressedData = NULL,
                                  unsigned int compressedDataSize = 0);
#endif
    virtual unsigned int getSizeOnDisk();

    virtual std::wstring getFilename();

    virtual std::vector<FileEntry*>* getFilesWithPrefix(
        const std::wstring& prefix);
    virtual std::vector<FileEntry*>* getRegionFilesByDimension(
        unsigned int dimensionIndex);

#if 0 || 0 || 0
    virtual std::wstring getPlayerDataFilenameForLoad(const PlayerUID& pUID);
    virtual std::wstring getPlayerDataFilenameForSave(const PlayerUID& pUID);
    virtual std::vector<FileEntry*>* getValidPlayerDatFiles();
#endif  //0

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
    virtual bool isLocalEndianDifferent(ESavePlatform plat);

    virtual void ConvertRegionFile(File sourceFile);
    virtual void ConvertToLocalPlatform();

protected:
    virtual void* getWritePointer(FileEntry* file);
};
