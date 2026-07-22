


#include "minecraft/world/level/storage/ConsoleSaveFileIO/FileHeader.h"

#include <assert.h>
#include <wchar.h>

#include <algorithm>
#include <compare>
#include <string>
#include <vector>

#include "app/mac/MacGame.h"
#include "app/mac/Stubs/winapi_stubs.h"
#include "util/Definitions.h"
#include "java/System.h"

extern MacGame app;

FileHeader::FileHeader() {
    lastFile = nullptr;
    m_saveVersion = 0;

    
    
    m_originalSaveVersion = SAVE_FILE_VERSION_NUMBER;
    m_savePlatform = SAVE_FILE_PLATFORM_LOCAL;
    m_saveEndian = m_localEndian;
}

FileHeader::~FileHeader() {
    for (unsigned int i = 0; i < fileTable.size(); ++i) {
        delete fileTable[i];
    }
}

FileEntry* FileHeader::AddFile(const std::wstring& name,
                               unsigned int length ) {
    assert(name.length() < 64);

    wchar_t filename[64];
    memset(&filename, 0, sizeof(wchar_t) * 64);
    memcpy(&filename, name.c_str(),
           std::min(sizeof(wchar_t) * 64, sizeof(wchar_t) * name.length()));

    
    
    for (unsigned int i = 0; i < fileTable.size(); ++i) {
        if (wcscmp(fileTable[i]->data.filename, filename) == 0) {
            
            return fileTable[i];
        }
    }

    
    fileTable.push_back(new FileEntry(filename, length, GetStartOfNextData()));
    lastFile = fileTable[fileTable.size() - 1];
    return lastFile;
}

void FileHeader::RemoveFile(FileEntry* file) {
    if (file == nullptr) return;

    AdjustStartOffsets(file, file->getFileSize(), true);

    auto it = find(fileTable.begin(), fileTable.end(), file);

    if (it < fileTable.end()) {
        fileTable.erase(it);
    }

#if !defined(_CONTENT_PACKAGE)
    wprintf(L"Removed file %ls\n", file->data.filename);
#endif

    delete file;
}

void FileHeader::WriteHeader(void* saveMem) {
    unsigned int headerOffset = GetStartOfNextData();

    
    
    unsigned int headerSize = (int)(fileTable.size());

    

    
    
    int* begin = (int*)saveMem;
    *begin = headerOffset;

    
    
    *(begin + 1) = headerSize;

    short* versions = (short*)(begin + 2);
    
    *versions = m_originalSaveVersion;

    
    short versionNumber = SAVE_FILE_VERSION_NUMBER;
    
    
    *(versions + 1) = versionNumber;

#if defined(_DEBUG_FILE_HEADER)
    app.DebugPrintf(
        "Write save file with original version: %d, and current version %d\n",
        m_originalSaveVersion, versionNumber);
#endif

    char* headerPosition = (char*)saveMem + headerOffset;

#if defined(_DEBUG_FILE_HEADER)
    app.DebugPrintf("\n\nWrite file Header: Offset = %d, Size = %d\n",
                    headerOffset, headerSize);
#endif

    
    for (unsigned int i = 0; i < fileTable.size(); ++i) {
        
        
        
        
        memcpy((void*)headerPosition, &fileTable[i]->data,
               sizeof(FileEntrySaveData));
        
        headerPosition += sizeof(FileEntrySaveData);
    }
}

void FileHeader::ReadHeader(
    void* saveMem, ESavePlatform plat ) {
    unsigned int headerOffset;
    unsigned int headerSize;

    m_savePlatform = plat;

    switch (m_savePlatform) {
        case SAVE_FILE_PLATFORM_X360:
        case SAVE_FILE_PLATFORM_PS3:
            m_saveEndian = std::endian::big;
            break;
        case SAVE_FILE_PLATFORM_XBONE:
        case SAVE_FILE_PLATFORM_WIN64:
        case SAVE_FILE_PLATFORM_PS4:
        case SAVE_FILE_PLATFORM_PSVITA:
            m_saveEndian = std::endian::little;
            break;
        default:
            assert(0);
            m_savePlatform = SAVE_FILE_PLATFORM_LOCAL;
            m_saveEndian = m_localEndian;
            break;
    }

    
    
    int* begin = (int*)saveMem;
    headerOffset = *begin;
    if (isSaveEndianDifferent()) System::ReverseULONG(&headerOffset);

    
    
    headerSize = *(begin + 1);
    if (isSaveEndianDifferent()) System::ReverseULONG(&headerSize);

    short* versions = (short*)(begin + 2);
    
    m_originalSaveVersion = *(versions);
    if (isSaveEndianDifferent()) System::ReverseSHORT(&m_originalSaveVersion);

    
    
    m_saveVersion = *(versions + 1);
    if (isSaveEndianDifferent()) System::ReverseSHORT(&m_saveVersion);

#if defined(_DEBUG_FILE_HEADER)
    app.DebugPrintf(
        "Read save file with orignal version: %d, and current version %d\n",
        m_originalSaveVersion, m_saveVersion);
    app.DebugPrintf("\n\nRead file Header: Offset = %d, Size = %d\n",
                    headerOffset, headerSize);
#endif

    char* headerPosition = (char*)saveMem + headerOffset;

    switch (m_saveVersion) {
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        case SAVE_FILE_VERSION_COMPRESSED_CHUNK_STORAGE:
        case SAVE_FILE_VERSION_DURANGO_CHANGE_MAP_DATA_MAPPING_SIZE:
        case SAVE_FILE_VERSION_CHANGE_MAP_DATA_MAPPING_SIZE:
        case SAVE_FILE_VERSION_MOVED_STRONGHOLD:
        case SAVE_FILE_VERSION_NEW_END:
        case SAVE_FILE_VERSION_POST_LAUNCH:
        case SAVE_FILE_VERSION_LAUNCH: {
            
            
            

            
            FileEntrySaveData* fesdHeaderPosition =
                (FileEntrySaveData*)headerPosition;
            for (unsigned int i = 0; i < headerSize; ++i) {
                FileEntry* entry = new FileEntry();
                

                memcpy(&entry->data, fesdHeaderPosition,
                       sizeof(FileEntrySaveData));

                if (isSaveEndianDifferent()) {
                    
                    
                    System::ReverseULONG(&entry->data.length);
                    System::ReverseULONG(&entry->data.startOffset);
                    System::ReverseULONGLONG(&entry->data.lastModifiedTime);
                }

                entry->currentFilePointer = entry->data.startOffset;
                lastFile = entry;
                fileTable.push_back(entry);
#if defined(_DEBUG_FILE_HEADER)
                app.DebugPrintf(
                    "File: %ls, Start = %d, Length = %d, End = %d, Timestamp = "
                    "%lld\n",
                    entry->data.filename, entry->data.startOffset,
                    entry->data.length,
                    entry->data.startOffset + entry->data.length,
                    entry->data.lastModifiedTime);
#endif

                fesdHeaderPosition++;
            }
        } break;

        
        
        
        case SAVE_FILE_VERSION_PRE_LAUNCH: {
            
            
            
            unsigned int i = 0;
            while (i < headerSize) {
                FileEntry* entry = new FileEntry();
                

                memcpy(&entry->data, headerPosition,
                       sizeof(FileEntrySaveDataV1));

                entry->currentFilePointer = entry->data.startOffset;
                lastFile = entry;
                fileTable.push_back(entry);
#if defined(_DEBUG_FILE_HEADER)
                app.DebugPrintf(
                    "File: %ls, Start = %d, Length = %d, End = %d\n",
                    entry->data.filename, entry->data.startOffset,
                    entry->data.length,
                    entry->data.startOffset + entry->data.length);
#endif

                i += sizeof(FileEntrySaveDataV1);
                headerPosition += sizeof(FileEntrySaveDataV1);
            }
        } break;
        default:
#if !defined(_CONTENT_PACKAGE)
            app.DebugPrintf("**********  Invalid save version %d\n",
                            m_saveVersion);
            __debugbreak();
#endif
            break;
    }
}

unsigned int FileHeader::GetStartOfNextData() {
    
    
    
    
    unsigned int totalBytesSoFar = SAVE_FILE_HEADER_SIZE;
    for (unsigned int i = 0; i < fileTable.size(); ++i) {
        if (fileTable[i]->getFileSize() > 0)
            totalBytesSoFar += fileTable[i]->getFileSize();
    }
    return totalBytesSoFar;
}

unsigned int FileHeader::GetFileSize() {
    return GetStartOfNextData() +
           (sizeof(FileEntrySaveData) * (unsigned int)fileTable.size());
}

void FileHeader::AdjustStartOffsets(FileEntry* file,
                                    unsigned int nNumberOfBytesToWrite,
                                    bool subtract ) {
    bool found = false;
    for (unsigned int i = 0; i < fileTable.size(); ++i) {
        if (found == true) {
            if (subtract) {
                fileTable[i]->data.startOffset -= nNumberOfBytesToWrite;
                fileTable[i]->currentFilePointer -= nNumberOfBytesToWrite;
            } else {
                fileTable[i]->data.startOffset += nNumberOfBytesToWrite;
                fileTable[i]->currentFilePointer += nNumberOfBytesToWrite;
            }
        } else if (fileTable[i] == file) {
            found = true;
        }
    }
}

bool FileHeader::fileExists(const std::wstring& name) {
    for (unsigned int i = 0; i < fileTable.size(); ++i) {
        if (wcscmp(fileTable[i]->data.filename, name.c_str()) == 0) {
            
            return true;
        }
    }
    return false;
}

std::vector<FileEntry*>* FileHeader::getFilesWithPrefix(
    const std::wstring& prefix) {
    std::vector<FileEntry*>* files = nullptr;

    for (unsigned int i = 0; i < fileTable.size(); ++i) {
        if (wcsncmp(fileTable[i]->data.filename, prefix.c_str(),
                    prefix.size()) == 0) {
            if (files == nullptr) {
                files = new std::vector<FileEntry*>();
            }

            files->push_back(fileTable[i]);
        }
    }

    return files;
}

std::endian FileHeader::getEndian(ESavePlatform plat) {
    std::endian platEndian;
    switch (plat) {
        case SAVE_FILE_PLATFORM_X360:
        case SAVE_FILE_PLATFORM_PS3:
            return std::endian::big;
            break;

        case SAVE_FILE_PLATFORM_NONE:
        case SAVE_FILE_PLATFORM_XBONE:
        case SAVE_FILE_PLATFORM_PS4:
        case SAVE_FILE_PLATFORM_PSVITA:
        case SAVE_FILE_PLATFORM_WIN64:
            return std::endian::little;
            break;
        default:
            assert(0);
            break;
    }
    return std::endian::little;
}