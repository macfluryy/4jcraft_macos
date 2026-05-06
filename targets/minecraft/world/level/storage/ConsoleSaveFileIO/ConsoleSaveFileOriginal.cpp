#include "minecraft/world/level/storage/ConsoleSaveFileIO/ConsoleSaveFileOriginal.h"

#include <assert.h>
#include <wchar.h>

#include <algorithm>
#include <chrono>
#include <compare>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <format>
#include <vector>

#include "platform/PlatformTypes.h"
#include "app/common/App_enums.h"
#include "app/common/src/BuildVer/BuildVer.h"
#include "app/common/src/GameRules/LevelGeneration/LevelGenerationOptions.h"
#include "app/mac/MacGame.h"
#include "app/mac/Stubs/winapi_stubs.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/compression.h"
#include "java/File.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "java/InputOutputStream/DataOutputStream.h"
#include "java/System.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/server/MinecraftServer.h"
#include "minecraft/server/level/ServerLevel.h"
#include "minecraft/world/level/chunk/storage/RegionFile.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/ConsoleSaveFile.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/ConsoleSavePath.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/FileHeader.h"
#include "minecraft/world/level/storage/LevelData.h"
#include "platform/IPlatformStorage.h"
#include "platform/PlatformServices.h"

#define RESERVE_ALLOCATION MEM_RESERVE
#define COMMIT_ALLOCATION MEM_COMMIT

unsigned int ConsoleSaveFileOriginal::pagesCommitted = 0;
void* ConsoleSaveFileOriginal::pvHeap = nullptr;

ConsoleSaveFileOriginal::ConsoleSaveFileOriginal(
    const std::wstring& fileName, void* pvSaveData /*= nullptr*/,
    unsigned int initialFileSize /*= 0*/, bool forceCleanSave /*= false*/,
    ESavePlatform plat /*= SAVE_FILE_PLATFORM_LOCAL*/) {
    if (pvHeap == nullptr) {
        pvHeap = VirtualAlloc(nullptr, MAX_PAGE_COUNT * CSF_PAGE_SIZE,
                              RESERVE_ALLOCATION, PAGE_READWRITE);
    }

    pvSaveMem = pvHeap;
    m_fileName = fileName;

    unsigned int fileSize = initialFileSize;

    bool bLevelGenBaseSave = false;
    LevelGenerationOptions* levelGen = app.getLevelGenerationOptions();
    if (pvSaveData == nullptr && levelGen != nullptr &&
        levelGen->requiresBaseSave()) {
        pvSaveData = levelGen->getBaseSaveData(fileSize);
        if (pvSaveData && fileSize != 0) bLevelGenBaseSave = true;
    }

    if (pvSaveData == nullptr || fileSize == 0)
        fileSize = PlatformStorage.GetSaveSize();

    if (forceCleanSave) fileSize = 0;

    unsigned int heapSize = std::max(
        fileSize,
        1024u * 1024u * 2u);
    if (pagesCommitted != 0) {
#ifndef _CONTENT_PACKAGE
        __debugbreak();
#endif
    }

    unsigned int pagesRequired =
        (heapSize + (CSF_PAGE_SIZE - 1)) / CSF_PAGE_SIZE;

    void* pvRet = VirtualAlloc(pvHeap, pagesRequired * CSF_PAGE_SIZE,
                               COMMIT_ALLOCATION, PAGE_READWRITE);
    if (pvRet == nullptr) {
#ifndef _CONTENT_PACKAGE
        __debugbreak();
#endif
    }
    pagesCommitted = pagesRequired;

    if (fileSize > 0) {
        if (pvSaveData != nullptr) {
            memcpy(pvSaveMem, pvSaveData, fileSize);
            if (bLevelGenBaseSave) {
                levelGen->deleteBaseSaveData();
            }
        } else {
            unsigned int storageLength;
            PlatformStorage.GetSaveData(pvSaveMem, &storageLength);
            app.DebugPrintf("Filesize - %d, Adjusted size - %d\n", fileSize,
                            storageLength);
            fileSize = storageLength;
        }
        void* pvSourceData = pvSaveMem;
        int compressed = *(int*)pvSourceData;
        if (compressed == 0) {
            unsigned int decompSize = *((int*)pvSourceData + 1);
            if (isLocalEndianDifferent(plat)) System::ReverseULONG(&decompSize);
            if (decompSize == 0) {
                app.DebugPrintf("Invalid save data format\n");
                std::memset(pvSourceData, 0, fileSize);
                header.WriteHeader(pvSourceData);
            } else {
                unsigned char* buf = new unsigned char[decompSize];
                Compression::getCompression()->SetDecompressionType(
                    plat);
                Compression::getCompression()->Decompress(
                    buf, &decompSize, (unsigned char*)pvSourceData + 8,
                    fileSize - 8);
                Compression::getCompression()->SetDecompressionType(
                    SAVE_FILE_PLATFORM_LOCAL);
                unsigned int currentHeapSize = pagesCommitted * CSF_PAGE_SIZE;

                unsigned int desiredSize = decompSize;

                if (desiredSize > currentHeapSize) {
                    unsigned int pagesRequired =
                        (desiredSize + (CSF_PAGE_SIZE - 1)) / CSF_PAGE_SIZE;
                    void* pvRet =
                        VirtualAlloc(pvHeap, pagesRequired * CSF_PAGE_SIZE,
                                     COMMIT_ALLOCATION, PAGE_READWRITE);
                    if (pvRet == nullptr) {
                        __debugbreak();
                    }
                    pagesCommitted = pagesRequired;
                }
                memcpy(pvSaveMem, buf, decompSize);
                delete[] buf;
            }
        }

        header.ReadHeader(pvSaveMem, plat);

    } else {
        header.WriteHeader(pvSaveMem);
    }
}

ConsoleSaveFileOriginal::~ConsoleSaveFileOriginal() {
    VirtualFree(pvHeap, MAX_PAGE_COUNT * CSF_PAGE_SIZE, MEM_DECOMMIT);
    pagesCommitted = 0;
}
FileEntry* ConsoleSaveFileOriginal::createFile(
    const ConsoleSavePath& fileName) {
    LockSaveAccess();
    FileEntry* file = header.AddFile(fileName.getName());
    ReleaseSaveAccess();

    return file;
}

void ConsoleSaveFileOriginal::deleteFile(FileEntry* file) {
    if (file == nullptr) return;

    LockSaveAccess();

    unsigned int numberOfBytesRead = 0;
    unsigned int numberOfBytesWritten = 0;

    const int bufferSize = 4096;
    int amountToRead = bufferSize;
    std::uint8_t buffer[bufferSize];
    unsigned int bufferDataSize = 0;

    char* readStartOffset =
        (char*)pvSaveMem + file->data.startOffset + file->getFileSize();

    char* writeStartOffset = (char*)pvSaveMem + file->data.startOffset;

    char* endOfDataOffset = (char*)pvSaveMem + header.GetStartOfNextData();

    while (true) {
        if (readStartOffset + bufferSize > endOfDataOffset) {
            amountToRead = (int)(endOfDataOffset - readStartOffset);
        } else {
            amountToRead = bufferSize;
        }

        if (amountToRead == 0) break;

        memcpy(buffer, readStartOffset, amountToRead);
        numberOfBytesRead = amountToRead;

        bufferDataSize = amountToRead;
        readStartOffset += numberOfBytesRead;

        memcpy((void*)writeStartOffset, buffer, bufferDataSize);
        numberOfBytesWritten = bufferDataSize;

        writeStartOffset += numberOfBytesWritten;
    }

    header.RemoveFile(file);

    finalizeWrite();

    ReleaseSaveAccess();
}

void ConsoleSaveFileOriginal::setFilePointer(FileEntry* file,
                                             unsigned int distanceToMove,
                                             SaveFileSeekOrigin seekOrigin) {
    LockSaveAccess();

    switch (seekOrigin) {
        case SaveFileSeekOrigin::Current:
            file->currentFilePointer += distanceToMove;
            break;
        case SaveFileSeekOrigin::End:
            file->currentFilePointer =
                file->data.startOffset + file->getFileSize() + distanceToMove;
            break;
        case SaveFileSeekOrigin::Begin:
        default:
            file->currentFilePointer = file->data.startOffset + distanceToMove;
            break;
    }

    ReleaseSaveAccess();
}
void ConsoleSaveFileOriginal::PrepareForWrite(
    FileEntry* file, unsigned int nNumberOfBytesToWrite) {
    int bytesToGrowBy = ((file->currentFilePointer - file->data.startOffset) +
                         nNumberOfBytesToWrite) -
                        file->getFileSize();
    if (bytesToGrowBy <= 0) return;
    MoveDataBeyond(file, bytesToGrowBy);
    if (file->data.length < 0) file->data.length = 0;
    file->data.length += bytesToGrowBy;
    finalizeWrite();
}

bool ConsoleSaveFileOriginal::writeFile(FileEntry* file, const void* lpBuffer,
                                        unsigned int nNumberOfBytesToWrite,
                                        unsigned int* lpNumberOfBytesWritten) {
    assert(pvSaveMem != nullptr);
    if (pvSaveMem == nullptr) {
        return false;
    }

    LockSaveAccess();

    PrepareForWrite(file, nNumberOfBytesToWrite);

    char* writeStartOffset = (char*)pvSaveMem + file->currentFilePointer;
    // printf("Write: pvSaveMem = %0xd, currentFilePointer = %d,
    // writeStartOffset = %0xd\n", pvSaveMem, file->currentFilePointer,
    // writeStartOffset);

    memcpy((void*)writeStartOffset, lpBuffer, nNumberOfBytesToWrite);
    *lpNumberOfBytesWritten = nNumberOfBytesToWrite;

    if (file->data.length < 0) file->data.length = 0;

    file->currentFilePointer += *lpNumberOfBytesWritten;

    // wprintf(L"Wrote %d bytes to %s, new file pointer is %I64d\n",
    // *lpNumberOfBytesWritten, file->data.filename, file->currentFilePointer);

    file->updateLastModifiedTime();

    ReleaseSaveAccess();

    return true;
}

bool ConsoleSaveFileOriginal::zeroFile(FileEntry* file,
                                       unsigned int nNumberOfBytesToWrite,
                                       unsigned int* lpNumberOfBytesWritten) {
    assert(pvSaveMem != nullptr);
    if (pvSaveMem == nullptr) {
        return false;
    }

    LockSaveAccess();

    PrepareForWrite(file, nNumberOfBytesToWrite);

    char* writeStartOffset = (char*)pvSaveMem + file->currentFilePointer;
    // printf("Write: pvSaveMem = %0xd, currentFilePointer = %d,
    // writeStartOffset = %0xd\n", pvSaveMem, file->currentFilePointer,
    // writeStartOffset);

    memset((void*)writeStartOffset, 0, nNumberOfBytesToWrite);
    *lpNumberOfBytesWritten = nNumberOfBytesToWrite;

    if (file->data.length < 0) file->data.length = 0;

    file->currentFilePointer += *lpNumberOfBytesWritten;

    // wprintf(L"Wrote %d bytes to %s, new file pointer is %I64d\n",
    // *lpNumberOfBytesWritten, file->data.filename, file->currentFilePointer);

    file->updateLastModifiedTime();

    ReleaseSaveAccess();

    return true;
}

bool ConsoleSaveFileOriginal::readFile(FileEntry* file, void* lpBuffer,
                                       unsigned int nNumberOfBytesToRead,
                                       unsigned int* lpNumberOfBytesRead) {
    unsigned int actualBytesToRead;
    assert(pvSaveMem != nullptr);
    if (pvSaveMem == nullptr) {
        return false;
    }

    LockSaveAccess();

    char* readStartOffset = (char*)pvSaveMem + file->currentFilePointer;
    // printf("Read: pvSaveMem = %0xd, currentFilePointer = %d, readStartOffset
    // = %0xd\n", pvSaveMem, file->currentFilePointer, readStartOffset);

    assert(nNumberOfBytesToRead <= file->getFileSize());

    actualBytesToRead = nNumberOfBytesToRead;
    if (file->currentFilePointer + nNumberOfBytesToRead >
        file->data.startOffset + file->data.length) {
        actualBytesToRead = (file->data.startOffset + file->data.length) -
                            file->currentFilePointer;
    }

    memcpy(lpBuffer, readStartOffset, actualBytesToRead);

    *lpNumberOfBytesRead = actualBytesToRead;

    file->currentFilePointer += *lpNumberOfBytesRead;

    // wprintf(L"Read %d bytes from %s, new file pointer is %I64d\n",
    // *lpNumberOfBytesRead, file->data.filename, file->currentFilePointer);

    ReleaseSaveAccess();

    return true;
}

bool ConsoleSaveFileOriginal::closeHandle(FileEntry* file) {
    LockSaveAccess();
    finalizeWrite();
    ReleaseSaveAccess();

    return true;
}

void ConsoleSaveFileOriginal::finalizeWrite() {
    LockSaveAccess();
    header.WriteHeader(pvSaveMem);
    ReleaseSaveAccess();
}

void ConsoleSaveFileOriginal::MoveDataBeyond(
    FileEntry* file, unsigned int nNumberOfBytesToWrite) {
    unsigned int numberOfBytesRead = 0;
    unsigned int numberOfBytesWritten = 0;

    const unsigned int bufferSize = 4096;
    unsigned int amountToRead = bufferSize;
    // assert( nNumberOfBytesToWrite <= bufferSize );
    static std::uint8_t buffer1[bufferSize];
    static std::uint8_t buffer2[bufferSize];
    unsigned int buffer1Size = 0;
    unsigned int buffer2Size = 0;
    unsigned int currentHeapSize = pagesCommitted * CSF_PAGE_SIZE;

    unsigned int desiredSize = header.GetFileSize() + nNumberOfBytesToWrite;

    if (desiredSize > currentHeapSize) {
        unsigned int pagesRequired =
            (desiredSize + (CSF_PAGE_SIZE - 1)) / CSF_PAGE_SIZE;
        void* pvRet = VirtualAlloc(pvHeap, pagesRequired * CSF_PAGE_SIZE,
                                   COMMIT_ALLOCATION, PAGE_READWRITE);
        if (pvRet == nullptr) {
            __debugbreak();
        }
        pagesCommitted = pagesRequired;
    }

    char* spaceStartOffset =
        (char*)pvSaveMem + file->data.startOffset + file->getFileSize();
    char* spaceEndOffset = spaceStartOffset + nNumberOfBytesToWrite;
    char* beginEndOfDataOffset = (char*)pvSaveMem + header.GetStartOfNextData();
    char* finishEndOfDataOffset = beginEndOfDataOffset + nNumberOfBytesToWrite;
    char* readStartOffset = beginEndOfDataOffset;
    char* writeStartOffset = finishEndOfDataOffset;

    // printf("\n******* MOVEDATABEYOND *******\n");
    // printf("Space start: %d, space end: %d\n", spaceStartOffset - (char
    // *)pvSaveMem, spaceEndOffset - (char *)pvSaveMem); printf("Current end of
    // data: %d, new end of data: %d\n", beginEndOfDataOffset - (char
    // *)pvSaveMem, finishEndOfDataOffset - (char *)pvSaveMem);
    if ((nNumberOfBytesToWrite & 4095) == 0) {
        if (nNumberOfBytesToWrite > 0) {
            uintptr_t uiFromStart = (uintptr_t)spaceStartOffset;
            uintptr_t uiFromEnd = (uintptr_t)beginEndOfDataOffset;
            uintptr_t uiFromStartChunk = uiFromStart & ~((uintptr_t)4095);
            uintptr_t uiFromEndChunk = (uiFromEnd - 1) & ~((uintptr_t)4095);
            for (uintptr_t uiCurrentChunk = uiFromEndChunk;
                 uiCurrentChunk >= uiFromStartChunk; uiCurrentChunk -= 4096) {
                uintptr_t uiCopyStart = uiCurrentChunk;
                uintptr_t uiCopyEnd = uiCurrentChunk + 4096;
                if (uiCopyStart < uiFromStart) {
                    uiCopyStart = uiFromStart;
                }
                if (uiCopyEnd > uiFromEnd) {
                    uiCopyEnd = uiFromEnd;
                }
                memcpy((void*)(uiCopyStart + nNumberOfBytesToWrite),
                       (void*)uiCopyStart, uiCopyEnd - uiCopyStart);
            }
        }
    } else {
        while (true) {
            memcpy(buffer2, buffer1, buffer1Size);
            buffer2Size = buffer1Size;

            if ((readStartOffset - bufferSize) < spaceStartOffset) {
                amountToRead = static_cast<unsigned int>(readStartOffset -
                                                         spaceStartOffset);
            } else {
                amountToRead = bufferSize;
            }
            readStartOffset -= amountToRead;

            // printf("About to read %u from %d\n", amountToRead,
            // readStartOffset - (char *)pvSaveMem );
            memcpy(buffer1, readStartOffset, amountToRead);
            numberOfBytesRead = amountToRead;

            buffer1Size = amountToRead;
            writeStartOffset -= buffer2Size;

            if ((writeStartOffset + buffer2Size) <= finishEndOfDataOffset) {
                // printf("About to write %u to %d\n", buffer2Size,
                // writeStartOffset - (char *)pvSaveMem );
                memcpy((void*)writeStartOffset, buffer2, buffer2Size);
                numberOfBytesWritten = buffer2Size;
            } else {
                assert((writeStartOffset + buffer2Size) <=
                       finishEndOfDataOffset);
                numberOfBytesWritten = 0;
            }

            if (numberOfBytesRead == 0) {
                // printf("\n************** MOVE COMPLETED ***************
                // \n\n");
                assert(writeStartOffset == spaceEndOffset);
                break;
            }
        }
    }

    header.AdjustStartOffsets(file, nNumberOfBytesToWrite);
}

bool ConsoleSaveFileOriginal::doesFileExist(ConsoleSavePath file) {
    LockSaveAccess();
    bool exists = header.fileExists(file.getName());
    ReleaseSaveAccess();

    return exists;
}

void ConsoleSaveFileOriginal::Flush(bool autosave, bool updateThumbnail) {
    LockSaveAccess();

    finalizeWrite();

    float fElapsedTime = 0.0f;

    unsigned int fileSize = header.GetFileSize();
    unsigned int compLength = fileSize + 8;
    std::uint8_t* compData =
        (std::uint8_t*)PlatformStorage.AllocateSaveData(compLength);

    if (compData == nullptr) {
        compLength = 0;

        const auto startTime = std::chrono::steady_clock::now();
        Compression::getCompression()->Compress(nullptr, &compLength, pvSaveMem,
                                                fileSize);
        fElapsedTime = std::chrono::duration<float>(
                           std::chrono::steady_clock::now() - startTime)
                           .count();

        app.DebugPrintf("Check buffer size: Elapsed time %f\n", fElapsedTime);

        compLength = compLength + 8;
        compData = (std::uint8_t*)PlatformStorage.AllocateSaveData(compLength);
    }

    if (compData != nullptr) {
        const auto startTime = std::chrono::steady_clock::now();
        Compression::getCompression()->Compress(compData + 8, &compLength,
                                                pvSaveMem, fileSize);
        fElapsedTime = std::chrono::duration<float>(
                           std::chrono::steady_clock::now() - startTime)
                           .count();

        app.DebugPrintf("Compress: Elapsed time %f\n", fElapsedTime);

        std::fill_n(compData, 8, std::uint8_t{0});
        int saveVer = 0;
        memcpy(compData, &saveVer, sizeof(int));
        memcpy(compData + 4, &fileSize, sizeof(int));

        app.DebugPrintf("Save data compressed from %d to %d\n", fileSize,
                        compLength);

        std::uint8_t* pbThumbnailData = nullptr;
        unsigned int dwThumbnailDataSize = 0;

        std::uint8_t* pbDataSaveImage = nullptr;
        unsigned int dwDataSizeSaveImage = 0;

#ifdef _WINDOWS64
        app.GetSaveThumbnail(&pbThumbnailData, &dwThumbnailDataSize,
                             &pbDataSaveImage, &dwDataSizeSaveImage);
#endif

        std::uint8_t bTextMetadata[88] = {};

        int64_t seed = 0;
        bool hasSeed = false;
        if (MinecraftServer::getInstance() != nullptr &&
            MinecraftServer::getInstance()->levels[0] != nullptr) {
            seed = MinecraftServer::getInstance()
                       ->levels[0]
                       ->getLevelData()
                       ->getSeed();
            hasSeed = true;
        }

        int iTextMetadataBytes = app.CreateImageTextData(
            bTextMetadata, seed, hasSeed,
            app.GetGameHostOption(eGameHostOption_All),
            Minecraft::GetInstance()->getCurrentTexturePackId());

        int32_t saveOrCheckpointId = 0;
        bool validSave =
            PlatformStorage.GetSaveUniqueNumber(&saveOrCheckpointId);
#ifdef _WINDOWS64
        PlatformStorage.SetSaveImages(pbThumbnailData, dwThumbnailDataSize,
                                      pbDataSaveImage, dwDataSizeSaveImage,
                                      bTextMetadata, iTextMetadataBytes);
        app.DebugPrintf("Save thumbnail size %d\n", dwThumbnailDataSize);

        PlatformStorage.SaveSaveData(
            &ConsoleSaveFileOriginal::SaveSaveDataCallback, this);
#ifndef _CONTENT_PACKAGE
        if (app.DebugSettingsOn()) {
            if (app.GetWriteSavesToFolderEnabled()) {
                DebugFlushToFile(compData, compLength + 8);
            }
        }
#endif
        ReleaseSaveAccess();
#else
        ReleaseSaveAccess();
#endif
    } else {
        ReleaseSaveAccess();
    }
}

#ifdef _WINDOWS64

int ConsoleSaveFileOriginal::SaveSaveDataCallback(void* lpParam, bool bRes) {
    ConsoleSaveFile* pClass = (ConsoleSaveFile*)lpParam;

    return 0;
}

#endif

#ifndef _CONTENT_PACKAGE
void ConsoleSaveFileOriginal::DebugFlushToFile(
    void* compressedData /*= nullptr*/,
    unsigned int compressedDataSize /*= 0*/) {
    LockSaveAccess();

    finalizeWrite();

    unsigned int fileSize = header.GetFileSize();

    unsigned int numberOfBytesWritten = 0;
    File targetFileDir = Minecraft::getSavesDirectory();

    if (!targetFileDir.exists()) targetFileDir.mkdirs();

    wchar_t* fileName = new wchar_t[XCONTENT_MAX_FILENAME_LENGTH + 1];

    std::time_t now = std::time(nullptr);
    std::tm t = *std::gmtime(&now);
    std::wstring cutFileName = m_fileName;
    if (m_fileName.length() > XCONTENT_MAX_FILENAME_LENGTH - 25) {
        cutFileName = m_fileName.substr(0, XCONTENT_MAX_FILENAME_LENGTH - 25);
    }
    swprintf(fileName, XCONTENT_MAX_FILENAME_LENGTH + 1,
             L"/v%04d-%ls%02d.%02d.%02d.%02d.%02d.mcs", VER_PRODUCTBUILD,
             cutFileName.c_str(), t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min,
             t.tm_sec);

    const std::wstring outputPath =
        targetFileDir.getPath() + std::wstring(fileName);
    bool writeSucceeded = false;

    if (compressedData != nullptr && compressedDataSize > 0) {
        writeSucceeded = PlatformFileIO.writeFile(
            outputPath, compressedData, compressedDataSize);
        numberOfBytesWritten = writeSucceeded ? compressedDataSize : 0;
        assert(numberOfBytesWritten == compressedDataSize);
    } else {
        writeSucceeded =
            PlatformFileIO.writeFile(outputPath, pvSaveMem, fileSize);
        numberOfBytesWritten = writeSucceeded ? fileSize : 0;
        assert(numberOfBytesWritten == fileSize);
    }

    delete[] fileName;

    ReleaseSaveAccess();
}
#endif

unsigned int ConsoleSaveFileOriginal::getSizeOnDisk() {
    return header.GetFileSize();
}

std::wstring ConsoleSaveFileOriginal::getFilename() { return m_fileName; }

std::vector<FileEntry*>* ConsoleSaveFileOriginal::getFilesWithPrefix(
    const std::wstring& prefix) {
    return header.getFilesWithPrefix(prefix);
}

std::vector<FileEntry*>* ConsoleSaveFileOriginal::getRegionFilesByDimension(
    unsigned int dimensionIndex) {
    return nullptr;
}

int ConsoleSaveFileOriginal::getSaveVersion() {
    return header.getSaveVersion();
}

int ConsoleSaveFileOriginal::getOriginalSaveVersion() {
    return header.getOriginalSaveVersion();
}

void ConsoleSaveFileOriginal::LockSaveAccess() { m_lock.lock(); }

void ConsoleSaveFileOriginal::ReleaseSaveAccess() { m_lock.unlock(); }

ESavePlatform ConsoleSaveFileOriginal::getSavePlatform() {
    return header.getSavePlatform();
}

bool ConsoleSaveFileOriginal::isSaveEndianDifferent() {
    return header.isSaveEndianDifferent();
}

void ConsoleSaveFileOriginal::setLocalPlatform() { header.setLocalPlatform(); }

void ConsoleSaveFileOriginal::setPlatform(ESavePlatform plat) {
    header.setPlatform(plat);
}

std::endian ConsoleSaveFileOriginal::getSaveEndian() {
    return header.getSaveEndian();
}

std::endian ConsoleSaveFileOriginal::getLocalEndian() {
    return header.getLocalEndian();
}

void ConsoleSaveFileOriginal::setEndian(std::endian endian) {
    header.setEndian(endian);
}

bool ConsoleSaveFileOriginal::isLocalEndianDifferent(ESavePlatform plat) {
    return getLocalEndian() != header.getEndian(plat);
}

void ConsoleSaveFileOriginal::ConvertRegionFile(File sourceFile) {
    unsigned int numberOfBytesWritten = 0;
    unsigned int numberOfBytesRead = 0;

    RegionFile sourceRegionFile(this, &sourceFile);

    for (unsigned int x = 0; x < 32; ++x) {
        for (unsigned int z = 0; z < 32; ++z) {
            DataInputStream* dis =
                sourceRegionFile.getChunkDataInputStream(x, z);

            if (dis) {
                std::vector<uint8_t> inData(1024 * 1024);
                int read = dis->read(inData);
                dis->close();
                dis->deleteChildStream();
                delete dis;

                DataOutputStream* dos =
                    sourceRegionFile.getChunkDataOutputStream(x, z);
                dos->write(inData, 0, read);

                dos->close();
                dos->deleteChildStream();
                delete dos;
            }
        }
    }
    sourceRegionFile
        .writeAllOffsets();
}

void ConsoleSaveFileOriginal::ConvertToLocalPlatform() {
    if (getSavePlatform() == SAVE_FILE_PLATFORM_LOCAL) {
        return;
    }
    std::vector<FileEntry*>* allFilesInSave =
        getFilesWithPrefix(std::wstring(L""));
    for (auto it = allFilesInSave->begin(); it < allFilesInSave->end(); ++it) {
        FileEntry* fe = *it;
        std::wstring fName(fe->data.filename);
        std::wstring suffix(L".mcr");
        if (fName.compare(fName.length() - suffix.length(), suffix.length(),
                          suffix) == 0) {
            app.DebugPrintf("Processing a region file: %ls\n", fName.c_str());
            ConvertRegionFile(File(fe->data.filename));
        } else {
            app.DebugPrintf("%ls is not a region file, ignoring\n",
                            fName.c_str());
        }
    }

    setLocalPlatform();
}

void* ConsoleSaveFileOriginal::getWritePointer(FileEntry* file) {
    return (char*)pvSaveMem + file->currentFilePointer;
    ;
}
