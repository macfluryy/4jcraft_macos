#include "minecraft/world/level/storage/ConsoleSaveFileIO/ConsoleSaveFileSplit.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include <algorithm>
#include <chrono>
#include <compare>
#include <ctime>
#include <format>
#include <thread>
#include <utility>

#include "platform/PlatformTypes.h"
#include "app/common/App_enums.h"
#include "app/common/src/BuildVer/BuildVer.h"
#include "app/common/src/GameRules/LevelGeneration/LevelGenerationOptions.h"
#include "app/mac/MacGame.h"
#include "app/mac/Stubs/winapi_stubs.h"
#include "util/Timer.h"
#include "util/StringHelpers.h"
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
#include "minecraft/world/level/storage/ConsoleSaveFileIO/ConsoleSaveFileConverter.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/ConsoleSavePath.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/FileHeader.h"
#include "minecraft/world/level/storage/LevelData.h"
#include "platform/IPlatformStorage.h"
#include "platform/PlatformServices.h"

class ProgressListener;

#define RESERVE_ALLOCATION MEM_RESERVE
#define COMMIT_ALLOCATION MEM_COMMIT

unsigned int ConsoleSaveFileSplit::pagesCommitted = 0;
void* ConsoleSaveFileSplit::pvHeap = nullptr;

ConsoleSaveFileSplit::RegionFileReference::RegionFileReference(
    int index, unsigned int regionIndex, unsigned int length ,
    unsigned char* data ) {
    fileEntry = new FileEntry();
    fileEntry->currentFilePointer = 0;
    fileEntry->data.length = 0;
    fileEntry->data.regionIndex = regionIndex;
    this->data = 0;
    this->index = index;
    this->dirty = false;
    this->dataCompressed = data;
    this->dataCompressedSize = length;
    this->lastWritten = 0;
}

ConsoleSaveFileSplit::RegionFileReference::~RegionFileReference() {
    free(data);
    delete fileEntry;
}

void ConsoleSaveFileSplit::RegionFileReference::Compress() {
    unsigned char* dataIn = data;
    unsigned char* dataInLast = data + fileEntry->data.length;
    unsigned int outputSize = 4;
    unsigned int runLength = 0;
    while (dataIn != dataInLast) {
        unsigned char thisByte = *dataIn++;
        if ((thisByte != 0) || (runLength == (65535 + 256))) {
            if (runLength != 0) {
                if (runLength < 256) {
                    outputSize += 2;
                } else {
                    outputSize += 4;
                }
                runLength = 0;
            }
            if (thisByte == 0) {
                runLength++;
            } else {
                outputSize++;
            }
        } else {
            runLength++;
        }
    }
    if (runLength != 0) {
        if (runLength < 256) {
            outputSize += 2;
        } else {
            outputSize += 4;
        }
        runLength = 0;
    }

    dataCompressed = (unsigned char*)malloc(outputSize);
    *((unsigned int*)dataCompressed) = fileEntry->data.length;
    unsigned char* dataOut = dataCompressed + 4;
    dataIn = data;

    while (dataIn != dataInLast) {
        unsigned char thisByte = *dataIn++;
        if ((thisByte != 0) || (runLength == (65535 + 256))) {
            if (runLength != 0) {
                if (runLength < 256) {
                    *dataOut++ = 0;
                    *dataOut++ = runLength;
                } else {
                    *dataOut++ = 0;
                    *dataOut++ = 0;
                    unsigned int largeRunLength = runLength - 256;
                    *dataOut++ = (largeRunLength >> 8) & 0xff;
                    *dataOut++ = (largeRunLength) & 0xff;
                }
                runLength = 0;
            }
            if (thisByte == 0) {
                runLength++;
            } else {
                *dataOut++ = thisByte;
            }
        } else {
            runLength++;
        }
    }
    if (runLength != 0) {
        if (runLength < 256) {
            *dataOut++ = 0;
            *dataOut++ = runLength;
        } else {
            *dataOut++ = 0;
            *dataOut++ = 0;
            unsigned int largeRunLength = runLength - 256;
            *dataOut++ = (largeRunLength >> 8) & 0xff;
            *dataOut++ = (largeRunLength) & 0xff;
        }
        runLength = 0;
    }
    assert((dataOut - dataCompressed) == outputSize);
    dataCompressedSize = outputSize;
}

void ConsoleSaveFileSplit::RegionFileReference::Decompress() {
    fileEntry->data.length = *((unsigned int*)dataCompressed);

    if (fileEntry->data.length > 1 * 1024 * 1024) {
        unsigned int uncompressedSize = 0;
        unsigned char* dataIn = dataCompressed + 4;
        unsigned char* dataInLast = dataCompressed + dataCompressedSize;

        while (dataIn != dataInLast) {
            unsigned char thisByte = *dataIn++;
            if (thisByte == 0) {
                thisByte = *dataIn++;
                if (thisByte == 0) {
                    unsigned int runLength = (*dataIn++) << 8;
                    runLength |= (*dataIn++);
                    runLength += 256;
                    uncompressedSize += runLength;
                } else {
                    unsigned int runLength = thisByte;
                    uncompressedSize += runLength;
                }
            } else {
                uncompressedSize++;
            }
        }

        if (fileEntry->data.length != uncompressedSize) {
            fileEntry->data.length = 0;
            assert(0);
            return;
        }
    }

    data = (unsigned char*)malloc(fileEntry->data.length);
    unsigned char* dataIn = dataCompressed + 4;
    unsigned char* dataInLast = dataCompressed + dataCompressedSize;
    unsigned char* dataOut = data;

    while (dataIn != dataInLast) {
        unsigned char thisByte = *dataIn++;
        if (thisByte == 0) {
            thisByte = *dataIn++;
            if (thisByte == 0) {
                unsigned int runLength = (*dataIn++) << 8;
                runLength |= (*dataIn++);
                runLength += 256;
                for (unsigned int i = 0; i < runLength; i++) {
                    *dataOut++ = 0;
                }
            } else {
                unsigned int runLength = thisByte;
                for (unsigned int i = 0; i < runLength; i++) {
                    *dataOut++ = 0;
                }
            }
        } else {
            *dataOut++ = thisByte;
        }
    }
    if ((dataOut - data) != fileEntry->data.length) {
        free(data);
        fileEntry->data.length = 0;
        data = nullptr;
        assert(0);
    }
}

unsigned int ConsoleSaveFileSplit::RegionFileReference::GetCompressedSize() {
    unsigned char* dataIn = data;
    unsigned char* dataInLast = data + fileEntry->data.length;

    unsigned int outputSize = 4;
    unsigned int runLength = 0;
    while (dataIn != dataInLast) {
        unsigned char thisByte = *dataIn++;
        if ((thisByte != 0) || (runLength == (65535 + 256))) {
            if (runLength != 0) {
                if (runLength < 256) {
                    outputSize += 2;
                } else {
                    outputSize += 4;
                }
                runLength = 0;
            }
            if (thisByte == 0) {
                runLength++;
            } else {
                outputSize++;
            }
        } else {
            runLength++;
        }
    }
    if (runLength != 0) {
        if (runLength < 256) {
            outputSize += 2;
        } else {
            outputSize += 4;
        }
        runLength = 0;
    }
    return outputSize;
}

void ConsoleSaveFileSplit::RegionFileReference::ReleaseCompressed() {
    free(dataCompressed);
    dataCompressed = nullptr;
    dataCompressedSize = 0;
}

FileEntry* ConsoleSaveFileSplit::GetRegionFileEntry(unsigned int regionIndex) {
    auto it = regionFiles.find(regionIndex);
    if (it != regionFiles.end()) {
        return it->second->fileEntry;
    }

    int index = PlatformStorage.AddSubfile(regionIndex);
    RegionFileReference* newRef = new RegionFileReference(index, regionIndex);
    regionFiles[regionIndex] = newRef;

    return newRef->fileEntry;
}

ConsoleSaveFileSplit::ConsoleSaveFileSplit(
    const std::wstring& fileName, void* pvSaveData ,
    unsigned int initialFileSize , bool forceCleanSave ,
    ESavePlatform plat ) {
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

    _init(fileName, pvSaveData, fileSize, plat);

    if (bLevelGenBaseSave) {
        levelGen->deleteBaseSaveData();
    }
}

ConsoleSaveFileSplit::ConsoleSaveFileSplit(ConsoleSaveFile* sourceSave,
                                           bool alreadySmallRegions,
                                           ProgressListener* progress) {
    _init(sourceSave->getFilename(), nullptr, 0, sourceSave->getSavePlatform());

    header.setOriginalSaveVersion(sourceSave->getOriginalSaveVersion());
    header.setSaveVersion(sourceSave->getSaveVersion());

    if (alreadySmallRegions) {
        std::vector<FileEntry*>* sourceFiles =
            sourceSave->getFilesWithPrefix(L"");

        unsigned int bytesWritten = 0;
        for (auto it = sourceFiles->begin(); it != sourceFiles->end(); ++it) {
            FileEntry* sourceEntry = *it;
            sourceSave->setFilePointer(sourceEntry, 0,
                                       SaveFileSeekOrigin::Begin);

            FileEntry* targetEntry =
                createFile(ConsoleSavePath(sourceEntry->data.filename));

            writeFile(targetEntry, sourceSave->getWritePointer(sourceEntry),
                      sourceEntry->getFileSize(), &bytesWritten);
        }

        delete sourceFiles;
    } else {
        ConsoleSaveFileConverter::ConvertSave(sourceSave, this, progress);
    }
}

void ConsoleSaveFileSplit::_init(const std::wstring& fileName, void* pvSaveData,
                                 unsigned int fileSize, ESavePlatform plat) {
    m_lastTickTime = 0;

    if (pvHeap == nullptr) {
        pvHeap = VirtualAlloc(nullptr, MAX_PAGE_COUNT * CSF_PAGE_SIZE,
                              RESERVE_ALLOCATION, PAGE_READWRITE);
    }

    pvSaveMem = pvHeap;
    m_fileName = fileName;

    unsigned int regionCount = PlatformStorage.GetSubfileCount();
    for (unsigned int i = 0; i < regionCount; i++) {
        unsigned int regionIndex;
        unsigned char* regionDataCompressed;
        unsigned int regionSizeCompressed;

        PlatformStorage.GetSubfileDetails(i, (int*)&regionIndex,
                                          (void**)&regionDataCompressed,
                                          &regionSizeCompressed);

        RegionFileReference* regionFileRef = new RegionFileReference(
            i, regionIndex, regionSizeCompressed, regionDataCompressed);
        if (regionSizeCompressed > 0) {
            regionFileRef->Decompress();
        } else {
            regionFileRef->fileEntry->data.length = 0;
        }
        regionFileRef->ReleaseCompressed();
        regionFiles[regionIndex] = regionFileRef;
    }

    unsigned int heapSize = std::max(fileSize, 1024u * 1024u * 2u);

    if (pagesCommitted != 0) {
#if !defined(_CONTENT_PACKAGE)
        __debugbreak();
#endif
    }

    unsigned int pagesRequired =
        (heapSize + (CSF_PAGE_SIZE - 1)) / CSF_PAGE_SIZE;

    void* pvRet = VirtualAlloc(pvHeap, pagesRequired * CSF_PAGE_SIZE,
                               COMMIT_ALLOCATION, PAGE_READWRITE);
    if (pvRet == nullptr) {
#if !defined(_CONTENT_PACKAGE)
        __debugbreak();
#endif
    }
    pagesCommitted = pagesRequired;

    if (fileSize > 0) {
        if (pvSaveData != nullptr) {
            memcpy(pvSaveMem, pvSaveData, fileSize);
        } else {
            unsigned int storageLength;
            PlatformStorage.GetSaveData(pvSaveMem, &storageLength);
            app.DebugPrintf("Filesize - %d, Adjusted size - %d\n", fileSize,
                            storageLength);
            fileSize = storageLength;
        }

        int compressed = *(int*)pvSaveMem;
        if (compressed == 0) {
            unsigned int decompSize = *((int*)pvSaveMem + 1);

            if (decompSize == 0) {
                app.DebugPrintf("Invalid save data format\n");
                memset(pvSaveMem, 0, fileSize);
                header.WriteHeader(pvSaveMem);
            } else {
                unsigned char* buf = new unsigned char[decompSize];

                if (Compression::getCompression()->Decompress(
                        buf, &decompSize, (unsigned char*)pvSaveMem + 8,
                        fileSize - 8) == 0) {
                    unsigned int currentHeapSize =
                        pagesCommitted * CSF_PAGE_SIZE;

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
                } else {
                    app.DebugPrintf("Failed to decompress save data!\n");
#if !defined(_CONTENT_PACKAGE)
                    __debugbreak();
#endif
                    memset(pvSaveMem, 0, fileSize);
                    header.WriteHeader(pvSaveMem);
                }

                delete[] buf;
            }
        }

        header.ReadHeader(pvSaveMem, plat);

    } else {
        header.WriteHeader(pvSaveMem);
    }
}

ConsoleSaveFileSplit::~ConsoleSaveFileSplit() {
    VirtualFree(pvHeap, MAX_PAGE_COUNT * CSF_PAGE_SIZE, MEM_DECOMMIT);
    pagesCommitted = 0;

    for (auto it = regionFiles.begin(); it != regionFiles.end(); it++) {
        delete it->second;
    }

    PlatformStorage.ResetSubfiles();
}

FileEntry* ConsoleSaveFileSplit::createFile(const ConsoleSavePath& fileName) {
    LockSaveAccess();

    unsigned int regionFileIndex;
    bool isRegionFile =
        GetNumericIdentifierFromName(fileName.getName(), &regionFileIndex);
    if (isRegionFile) {
        if (!header.fileExists(fileName.getName())) {
            FileEntry* file = GetRegionFileEntry(regionFileIndex);
            ReleaseSaveAccess();
            return file;
        }
    }

    FileEntry* file = header.AddFile(fileName.getName());
    ReleaseSaveAccess();

    return file;
}

void ConsoleSaveFileSplit::deleteFile(FileEntry* file) {
    if (file == nullptr) return;

    assert(file->isRegionFile() == false);

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

void ConsoleSaveFileSplit::setFilePointer(FileEntry* file,
                                          unsigned int distanceToMove,
                                          SaveFileSeekOrigin seekOrigin) {
    LockSaveAccess();

    if (seekOrigin == SaveFileSeekOrigin::Current) {
        file->currentFilePointer += distanceToMove;
    } else {
        if (file->isRegionFile()) {
            file->currentFilePointer = distanceToMove;
        } else {
            file->currentFilePointer = file->data.startOffset + distanceToMove;
        }

        if (seekOrigin == SaveFileSeekOrigin::End) {
            file->currentFilePointer += file->getFileSize();
        }
    }

    ReleaseSaveAccess();
}

void ConsoleSaveFileSplit::PrepareForWrite(FileEntry* file,
                                           unsigned int nNumberOfBytesToWrite) {
    int bytesToGrowBy = ((file->currentFilePointer - file->data.startOffset) +
                         nNumberOfBytesToWrite) -
                        file->getFileSize();
    if (bytesToGrowBy <= 0) return;

    MoveDataBeyond(file, bytesToGrowBy);

    if (file->data.length < 0) file->data.length = 0;
    file->data.length += bytesToGrowBy;

    finalizeWrite();
}

bool ConsoleSaveFileSplit::writeFile(FileEntry* file, const void* lpBuffer,
                                     unsigned int nNumberOfBytesToWrite,
                                     unsigned int* lpNumberOfBytesWritten) {
    assert(pvSaveMem != nullptr);
    if (pvSaveMem == nullptr) {
        return false;
    }

    LockSaveAccess();

    if (file->isRegionFile()) {
        unsigned int sizeRequired =
            file->currentFilePointer + nNumberOfBytesToWrite;
        RegionFileReference* fileRef = regionFiles[file->data.regionIndex];
        if (sizeRequired > file->getFileSize()) {
            fileRef->data =
                (unsigned char*)realloc(fileRef->data, sizeRequired);
            file->data.length = sizeRequired;
        }

        memcpy(fileRef->data + file->currentFilePointer, lpBuffer,
               nNumberOfBytesToWrite);

        file->currentFilePointer += nNumberOfBytesToWrite;
        file->updateLastModifiedTime();
        fileRef->dirty = true;
    } else {
        PrepareForWrite(file, nNumberOfBytesToWrite);

        char* writeStartOffset = (char*)pvSaveMem + file->currentFilePointer;

        memcpy((void*)writeStartOffset, lpBuffer, nNumberOfBytesToWrite);
        *lpNumberOfBytesWritten = nNumberOfBytesToWrite;

        if (file->data.length < 0) file->data.length = 0;

        file->currentFilePointer += *lpNumberOfBytesWritten;

        file->updateLastModifiedTime();
    }

    ReleaseSaveAccess();

    return true;
}

bool ConsoleSaveFileSplit::zeroFile(FileEntry* file,
                                    unsigned int nNumberOfBytesToWrite,
                                    unsigned int* lpNumberOfBytesWritten) {
    assert(pvSaveMem != nullptr);
    if (pvSaveMem == nullptr) {
        return false;
    }

    if (nNumberOfBytesToWrite == 0) {
        if (lpNumberOfBytesWritten) {
            *lpNumberOfBytesWritten = 0;
        }
        return 1;
    }

    LockSaveAccess();

    if (file->isRegionFile()) {
        unsigned int sizeRequired =
            file->currentFilePointer + nNumberOfBytesToWrite;
        RegionFileReference* fileRef = regionFiles[file->data.regionIndex];
        if (sizeRequired > file->getFileSize()) {
            fileRef->data =
                (unsigned char*)realloc(fileRef->data, sizeRequired);
            file->data.length = sizeRequired;
        }

        memset(fileRef->data + file->currentFilePointer, 0,
               nNumberOfBytesToWrite);

        file->currentFilePointer += nNumberOfBytesToWrite;
        file->updateLastModifiedTime();
        fileRef->dirty = true;
    } else {
        PrepareForWrite(file, nNumberOfBytesToWrite);

        char* writeStartOffset = (char*)pvSaveMem + file->currentFilePointer;

        memset((void*)writeStartOffset, 0, nNumberOfBytesToWrite);
        *lpNumberOfBytesWritten = nNumberOfBytesToWrite;

        if (file->data.length < 0) file->data.length = 0;

        file->currentFilePointer += *lpNumberOfBytesWritten;

        file->updateLastModifiedTime();
    }

    ReleaseSaveAccess();

    return true;
}

bool ConsoleSaveFileSplit::readFile(FileEntry* file, void* lpBuffer,
                                    unsigned int nNumberOfBytesToRead,
                                    unsigned int* lpNumberOfBytesRead) {
    unsigned int actualBytesToRead;
    assert(pvSaveMem != nullptr);
    if (pvSaveMem == nullptr) {
        return false;
    }

    LockSaveAccess();

    if (file->isRegionFile()) {
        actualBytesToRead = nNumberOfBytesToRead;
        if (file->currentFilePointer + nNumberOfBytesToRead >
            file->data.length) {
            actualBytesToRead = file->data.length - file->currentFilePointer;
        }
        RegionFileReference* fileRef = regionFiles[file->data.regionIndex];
        memcpy(lpBuffer, fileRef->data + file->currentFilePointer,
               actualBytesToRead);
        *lpNumberOfBytesRead = actualBytesToRead;

        file->currentFilePointer += actualBytesToRead;
    } else {
        char* readStartOffset = (char*)pvSaveMem + file->currentFilePointer;

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
    }

    ReleaseSaveAccess();

    return true;
}

bool ConsoleSaveFileSplit::closeHandle(FileEntry* file) {
    LockSaveAccess();
    finalizeWrite();
    ReleaseSaveAccess();

    return true;
}

void ConsoleSaveFileSplit::tick() {
    std::int64_t currentTime = System::currentTimeMillis();

    if (PlatformStorage.GetSaveState() != IPlatformStorage::ESaveGame_Idle) {
        return;
    }

    if (PlatformStorage.GetSaveDisabled()) {
        return;
    }

    if ((currentTime - m_lastTickTime) < WRITE_TICK_RATE_MS) {
        return;
    }

    LockSaveAccess();

    m_lastTickTime = currentTime;

    unsigned int bytesWritten = 0;
    for (auto it = writeHistory.begin(); it != writeHistory.end();) {
        if ((currentTime - it->writeTime) >
            (WRITE_BANDWIDTH_MEASUREMENT_PERIOD_SECONDS * 1000)) {
            it = writeHistory.erase(it);
        } else {
            bytesWritten += it->writeSize;
            it++;
        }
    }

    std::vector<DirtyRegionFile> dirtyRegions;
    for (auto it = regionFiles.begin(); it != regionFiles.end(); it++) {
        DirtyRegionFile dirtyRegion;

        if (it->second->dirty) {
            dirtyRegion.fileRef = it->second->fileEntry->getRegionFileIndex();
            dirtyRegion.lastWritten = it->second->lastWritten;
            dirtyRegions.push_back(dirtyRegion);
        }
    }

    std::sort(dirtyRegions.begin(), dirtyRegions.end());

    bool writeRequired = false;
    unsigned int bytesInTimePeriod = bytesWritten;
    unsigned int bytesAddedThisTick = 0;
    for (int i = 0; i < dirtyRegions.size(); i++) {
        RegionFileReference* regionRef = regionFiles[dirtyRegions[i].fileRef];
        unsigned int compressedSize = regionRef->GetCompressedSize();
        bytesInTimePeriod += compressedSize;
        bytesAddedThisTick += compressedSize;

        if ((i > 0) && (bytesAddedThisTick > WRITE_MAX_WRITE_PER_TICK)) {
            break;
        }

        if ((bytesInTimePeriod / WRITE_BANDWIDTH_MEASUREMENT_PERIOD_SECONDS) >
            WRITE_BANDWIDTH_BYTESPERSECOND) {
            break;
        }

        WriteHistory writeEvent;
        writeEvent.writeSize = compressedSize;
        writeEvent.writeTime = System::currentTimeMillis();
        writeHistory.push_back(writeEvent);

        regionRef->Compress();
        PlatformStorage.UpdateSubfile(regionRef->index,
                                      regionRef->dataCompressed,
                                      regionRef->dataCompressedSize);
        regionRef->dirty = false;
        regionRef->lastWritten = System::currentTimeMillis();

        writeRequired = true;
    }
#if !defined(_CONTENT_PACKAGE)
    {
        unsigned int totalDirty = 0;
        unsigned int totalDirtyBytes = 0;
        int64_t oldestDirty = currentTime;
        for (auto it = regionFiles.begin(); it != regionFiles.end(); it++) {
            if (it->second->dirty) {
                if (it->second->lastWritten < oldestDirty) {
                    oldestDirty = it->second->lastWritten;
                }
                totalDirty++;
                totalDirtyBytes += it->second->fileEntry->getFileSize();
            }
        }
    }
#endif

    if (writeRequired) {
        PlatformStorage.SaveSubfiles([this](bool bRes) {
            return SaveRegionFilesCallback(this, bRes);
        });
    }

    ReleaseSaveAccess();
}

void ConsoleSaveFileSplit::finalizeWrite() {
    LockSaveAccess();
    header.WriteHeader(pvSaveMem);
    ReleaseSaveAccess();
}

void ConsoleSaveFileSplit::MoveDataBeyond(FileEntry* file,
                                          unsigned int nNumberOfBytesToWrite) {
    unsigned int numberOfBytesRead = 0;
    unsigned int numberOfBytesWritten = 0;

    const unsigned int bufferSize = 4096;
    unsigned int amountToRead = bufferSize;
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

            memcpy(buffer1, readStartOffset, amountToRead);
            numberOfBytesRead = amountToRead;

            buffer1Size = amountToRead;

            writeStartOffset -= buffer2Size;

            if ((writeStartOffset + buffer2Size) <= finishEndOfDataOffset) {
                memcpy((void*)writeStartOffset, buffer2, buffer2Size);
                numberOfBytesWritten = buffer2Size;
            } else {
                assert((writeStartOffset + buffer2Size) <=
                       finishEndOfDataOffset);
                numberOfBytesWritten = 0;
            }

            if (numberOfBytesRead == 0) {
                assert(writeStartOffset == spaceEndOffset);
                break;
            }
        }
    }

    header.AdjustStartOffsets(file, nNumberOfBytesToWrite);
}

bool ConsoleSaveFileSplit::GetNumericIdentifierFromName(
    const std::wstring& fileName, unsigned int* idOut) {
    if (fileName.length() < 4) return false;
    std::wstring extension = fileName.substr(fileName.length() - 4, 4);
    if (extension != std::wstring(L".mcr")) return false;

    unsigned int id = 0;
    int x, z;

    const wchar_t* cstr = fileName.c_str();
    const wchar_t* body = cstr + 2;

    if (cstr[0] != L'r') {
        body = cstr + 7;
        if (cstr[3] == L'-') {
            id = 0x00010000;
        } else {
            id = 0x00020000;
        }
    }
    swscanf(body, L"%d.%d.mcr", &x, &z);

    id |= (((unsigned int)x << 8) & 0x0000ff00);
    id |= (z & 0x000000ff);

    *idOut = id;

    return true;
}

std::wstring ConsoleSaveFileSplit::GetNameFromNumericIdentifier(
    unsigned int idIn) {
    std::wstring prefix;

    switch ((idIn >> 16) & 0xff) {
        case 0:
            prefix = L"";
            break;
        case 1:
            prefix = L"DIM-1";
            break;
        case 2:
            prefix = L"DIM1/";
            break;
    }
    signed char regionX = (idIn >> 8) & 255;
    signed char regionZ = idIn & 255;
    std::wstring region = (prefix + std::wstring(L"r.") + toWString(regionX) +
                           L"." + toWString(regionZ) + L".mcr");

    return region;
}

void ConsoleSaveFileSplit::processSubfilesForWrite() {
    for (auto it = regionFiles.begin(); it != regionFiles.end(); it++) {
        RegionFileReference* region = it->second;
        if (region->dirty) {
            region->Compress();
            PlatformStorage.UpdateSubfile(region->index, region->dataCompressed,
                                          region->dataCompressedSize);
            region->dirty = false;
            region->lastWritten = System::currentTimeMillis();
        }
    }
}

void ConsoleSaveFileSplit::processSubfilesAfterWrite() {
    for (auto it = regionFiles.begin(); it != regionFiles.end(); it++) {
        RegionFileReference* region = it->second;
        region->ReleaseCompressed();
    }
}

bool ConsoleSaveFileSplit::doesFileExist(ConsoleSavePath file) {
    LockSaveAccess();
    bool exists = header.fileExists(file.getName());
    ReleaseSaveAccess();

    return exists;
}

void ConsoleSaveFileSplit::Flush(bool autosave, bool updateThumbnail) {
    LockSaveAccess();

    while (PlatformStorage.GetSaveState() != IPlatformStorage::ESaveGame_Idle) {
        app.DebugPrintf("Flush wait\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    finalizeWrite();

    m_autosave = autosave;
    if (!m_autosave) processSubfilesForWrite();

    time_util::Timer timer;

    unsigned int fileSize = header.GetFileSize();

    unsigned int compLength = fileSize + 8;

    std::uint8_t* compData =
        (std::uint8_t*)PlatformStorage.AllocateSaveData(compLength);

    if (compData == nullptr) {
        compLength = 0;

        timer.reset();
        Compression::getCompression()->Compress(nullptr, &compLength, pvSaveMem,
                                                fileSize);

        app.DebugPrintf("Check buffer size: Elapsed time %f\n",
                        static_cast<float>(timer.elapsed_seconds()));

        compLength = compLength + 8;

        compData = (std::uint8_t*)PlatformStorage.AllocateSaveData(compLength);
    }

    if (compData != nullptr) {
        timer.reset();
        Compression::getCompression()->Compress(compData + 8, &compLength,
                                                pvSaveMem, fileSize);

        app.DebugPrintf("Compress: Elapsed time %f\n",
                        static_cast<float>(timer.elapsed_seconds()));

        memset(compData, 0, 8);
        int saveVer = 0;
        memcpy(compData, &saveVer, sizeof(int));
        memcpy(compData + 4, &fileSize, sizeof(int));

        app.DebugPrintf("Save data compressed from %d to %d\n", fileSize,
                        compLength);

        if (updateThumbnail) {
            std::uint8_t* pbThumbnailData = nullptr;
            unsigned int dwThumbnailDataSize = 0;

            std::uint8_t* pbDataSaveImage = nullptr;
            unsigned int dwDataSizeSaveImage = 0;

            std::uint8_t bTextMetadata[88];
            memset(bTextMetadata, 0, 88);

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

            PlatformStorage.SetSaveImages(pbThumbnailData, dwThumbnailDataSize,
                                          pbDataSaveImage, dwDataSizeSaveImage,
                                          bTextMetadata, iTextMetadataBytes);
            app.DebugPrintf("Save thumbnail size %d\n", dwThumbnailDataSize);
        }

        int32_t saveOrCheckpointId = 0;
        bool validSave =
            PlatformStorage.GetSaveUniqueNumber(&saveOrCheckpointId);

        PlatformStorage.SaveSaveData([this](bool bRes) {
            return SaveSaveDataCallback(this, bRes);
        });
#if !defined(_CONTENT_PACKAGE)
        if (app.DebugSettingsOn()) {
            if (app.GetWriteSavesToFolderEnabled()) {
                DebugFlushToFile(compData, compLength + 8);
            }
        }
#endif

        WriteEntriesAsFolderToDisk(m_fileName);

        ReleaseSaveAccess();
    }
}

void ConsoleSaveFileSplit::WriteEntriesAsFolderToDisk(
    const std::wstring& worldName) {
    if (worldName.empty()) {
        app.DebugPrintf(
            "[WriteEntriesAsFolderToDisk] skip: empty world name\n");
        return;
    }
    if (pvSaveMem == nullptr) return;

    LockSaveAccess();

    File savesDir = Minecraft::getSavesDirectory();
    if (!savesDir.exists()) savesDir.mkdirs();

    File worldDir(savesDir, worldName);
    if (!worldDir.exists()) worldDir.mkdirs();

    int writtenCount = 0;
    for (size_t i = 0; i < header.fileTable.size(); ++i) {
        FileEntry* entry = header.fileTable[i];
        if (entry == nullptr) continue;
        if (entry->isRegionFile()) continue;

        std::wstring entryName(entry->data.filename);
        if (entryName.empty()) continue;

        unsigned int len = entry->getFileSize();
        const void* src =
            (const char*)pvSaveMem + entry->data.startOffset;

        File outFile(worldDir, entryName);
        std::wstring parentPath = outFile.getPath();
        size_t lastSlash = parentPath.find_last_of(L'/');
        if (lastSlash != std::wstring::npos) {
            File parentDir(parentPath.substr(0, lastSlash));
            if (!parentDir.exists()) parentDir.mkdirs();
        }

        std::filesystem::path fsPath(outFile.getPath());
        if (PlatformFileIO.writeFile(fsPath, src, len)) {
            ++writtenCount;
        } else {
            app.DebugPrintf(
                "[WriteEntriesAsFolderToDisk] failed to write %ls\n",
                outFile.getPath().c_str());
        }
    }

    int regionsWritten = 0;
    for (auto it = regionFiles.begin(); it != regionFiles.end(); ++it) {
        RegionFileReference* region = it->second;
        if (region == nullptr || region->data == nullptr) continue;
        unsigned int len = region->fileEntry->data.length;
        if (len == 0) continue;

        std::wstring regionName =
            GetNameFromNumericIdentifier(region->fileEntry->data.regionIndex);
        if (regionName.empty()) continue;

        File outFile(worldDir, regionName);
        std::wstring outPath = outFile.getPath();
        size_t lastSlash = outPath.find_last_of(L'/');
        if (lastSlash != std::wstring::npos) {
            File parentDir(outPath.substr(0, lastSlash));
            if (!parentDir.exists()) parentDir.mkdirs();
        }

        std::filesystem::path fsPath(outPath);
        if (PlatformFileIO.writeFile(fsPath, region->data, len)) {
            ++regionsWritten;
        } else {
            app.DebugPrintf(
                "[WriteEntriesAsFolderToDisk] failed region %ls\n",
                outPath.c_str());
        }
    }

    app.DebugPrintf(
        "[WriteEntriesAsFolderToDisk] wrote %d entries + %d regions to %ls\n",
        writtenCount, regionsWritten, worldDir.getPath().c_str());

    ReleaseSaveAccess();
}

namespace {
void _CollectEntriesUnder(const std::filesystem::path& root,
                          const std::filesystem::path& current,
                          std::vector<std::pair<std::wstring,
                                                std::filesystem::path>>& out) {
    std::error_code ec;
    for (auto it = std::filesystem::directory_iterator(current, ec);
         !ec && it != std::filesystem::directory_iterator(); it.increment(ec)) {
        const auto& p = it->path();
        std::error_code ec2;
        if (std::filesystem::is_directory(p, ec2)) {
            _CollectEntriesUnder(root, p, out);
        } else if (std::filesystem::is_regular_file(p, ec2)) {
            std::filesystem::path rel = std::filesystem::relative(p, root, ec2);
            if (ec2) continue;
            std::string s = rel.generic_string();
            if (s == ".DS_Store" || s.find("/.DS_Store") != std::string::npos)
                continue;
            std::wstring w(s.begin(), s.end());
            out.emplace_back(std::move(w), p);
        }
    }
}
}  

int ConsoleSaveFileSplit::ReadEntriesFromFolderOnDisk(
    const std::wstring& worldName) {
    if (worldName.empty()) return 0;

    File savesDir = Minecraft::getSavesDirectory();
    File worldDir(savesDir, worldName);
    if (!worldDir.exists() || !worldDir.isDirectory()) {
        app.DebugPrintf(
            "[ReadEntriesFromFolderOnDisk] no folder for %ls\n",
            worldName.c_str());
        return 0;
    }

    std::filesystem::path root(worldDir.getPath());
    std::vector<std::pair<std::wstring, std::filesystem::path>> files;
    _CollectEntriesUnder(root, root, files);

    int count = 0;
    for (auto& [name, fsPath] : files) {
        std::vector<std::uint8_t> bytes = PlatformFileIO.readFileToVec(fsPath);
        if (bytes.empty()) continue;

        ConsoleSavePath path(name);
        FileEntry* entry = createFile(path);
        if (entry == nullptr) continue;

        setFilePointer(entry, 0, SaveFileSeekOrigin::Begin);
        unsigned int written = 0;
        writeFile(entry, bytes.data(), (unsigned int)bytes.size(), &written);
        ++count;
    }

    app.DebugPrintf(
        "[ReadEntriesFromFolderOnDisk] imported %d entries from %ls\n",
        count, worldDir.getPath().c_str());

    return count;
}

int ConsoleSaveFileSplit::SaveSaveDataCallback(void* lpParam, bool bRes) {
    ConsoleSaveFileSplit* pClass = (ConsoleSaveFileSplit*)lpParam;

    if (!pClass->m_autosave) {
        PlatformStorage.SaveSubfiles([pClass](bool bRes) {
            return SaveRegionFilesCallback(pClass, bRes);
        });
    }
    return 0;
}

int ConsoleSaveFileSplit::SaveRegionFilesCallback(void* lpParam, bool bRes) {
    ConsoleSaveFileSplit* pClass = (ConsoleSaveFileSplit*)lpParam;
    pClass->processSubfilesAfterWrite();

    return 0;
}

#if !defined(_CONTENT_PACKAGE)
void ConsoleSaveFileSplit::DebugFlushToFile(
    void* compressedData ,
    unsigned int compressedDataSize ) {
    LockSaveAccess();

    finalizeWrite();

    unsigned int fileSize = header.GetFileSize();

    unsigned int numberOfBytesWritten = 0;

    File targetFileDir = Minecraft::getSavesDirectory();

    if (!targetFileDir.exists()) targetFileDir.mkdirs();

    wchar_t* fileName = new wchar_t[XCONTENT_MAX_FILENAME_LENGTH + 1];

    auto now_tp = std::chrono::system_clock::now();
    std::time_t now_tt = std::chrono::system_clock::to_time_t(now_tp);
    std::tm t{};
#if defined(_WIN32)
    gmtime_s(&t, &now_tt);
#else
    gmtime_r(&now_tt, &t);
#endif
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

unsigned int ConsoleSaveFileSplit::getSizeOnDisk() {
    return header.GetFileSize();
}

std::wstring ConsoleSaveFileSplit::getFilename() { return m_fileName; }

std::vector<FileEntry*>* ConsoleSaveFileSplit::getFilesWithPrefix(
    const std::wstring& prefix) {
    return header.getFilesWithPrefix(prefix);
}

std::vector<FileEntry*>* ConsoleSaveFileSplit::getRegionFilesByDimension(
    unsigned int dimensionIndex) {
    std::vector<FileEntry*>* files = nullptr;

    for (auto it = regionFiles.begin(); it != regionFiles.end(); ++it) {
        unsigned int entryDimension = ((it->first) >> 16) & 0xFF;

        if (entryDimension == dimensionIndex) {
            if (files == nullptr) {
                files = new std::vector<FileEntry*>();
            }

            files->push_back(it->second->fileEntry);
        }
    }

    return files;
}

int ConsoleSaveFileSplit::getSaveVersion() { return header.getSaveVersion(); }

int ConsoleSaveFileSplit::getOriginalSaveVersion() {
    return header.getOriginalSaveVersion();
}

void ConsoleSaveFileSplit::LockSaveAccess() { m_lock.lock(); }

void ConsoleSaveFileSplit::ReleaseSaveAccess() { m_lock.unlock(); }

ESavePlatform ConsoleSaveFileSplit::getSavePlatform() {
    return header.getSavePlatform();
}

bool ConsoleSaveFileSplit::isSaveEndianDifferent() {
    return header.isSaveEndianDifferent();
}

void ConsoleSaveFileSplit::setLocalPlatform() { header.setLocalPlatform(); }

void ConsoleSaveFileSplit::setPlatform(ESavePlatform plat) {
    header.setPlatform(plat);
}

std::endian ConsoleSaveFileSplit::getSaveEndian() {
    return header.getSaveEndian();
}

std::endian ConsoleSaveFileSplit::getLocalEndian() {
    return header.getLocalEndian();
}

void ConsoleSaveFileSplit::setEndian(std::endian endian) {
    header.setEndian(endian);
}

void ConsoleSaveFileSplit::ConvertRegionFile(File sourceFile) {
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
    sourceRegionFile.writeAllOffsets();
}

void ConsoleSaveFileSplit::ConvertToLocalPlatform() {
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