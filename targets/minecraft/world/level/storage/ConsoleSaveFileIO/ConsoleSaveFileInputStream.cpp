#include "minecraft/world/level/storage/ConsoleSaveFileIO/ConsoleSaveFileInputStream.h"

#include <assert.h>

#include <cstdint>
#include <vector>

#include "minecraft/world/level/storage/ConsoleSaveFileIO/ConsoleSaveFile.h"

class ConsoleSavePath;

ConsoleSaveFileInputStream::ConsoleSaveFileInputStream(
    ConsoleSaveFile* saveFile, const ConsoleSavePath& file) {
    m_saveFile = saveFile;
    m_file = m_saveFile->createFile(file);

    m_saveFile->setFilePointer(m_file, 0, SaveFileSeekOrigin::Begin);
}

ConsoleSaveFileInputStream::ConsoleSaveFileInputStream(
    ConsoleSaveFile* saveFile, FileEntry* file) {
    m_saveFile = saveFile;
    m_file = file;

    m_saveFile->setFilePointer(m_file, 0, SaveFileSeekOrigin::Begin);
}




int ConsoleSaveFileInputStream::read() {
    std::uint8_t byteRead = static_cast<std::uint8_t>(0);
    unsigned int numberOfBytesRead;

    bool result =
        m_saveFile->readFile(m_file,
                             &byteRead,          
                             1,                  
                             &numberOfBytesRead  
        );

    if (!result) {
        
        return -1;
    } else if (numberOfBytesRead == 0) {
        
        return -1;
    }

    return static_cast<int>(byteRead);
}






int ConsoleSaveFileInputStream::read(std::vector<uint8_t>& b) {
    unsigned int numberOfBytesRead;

    bool result =
        m_saveFile->readFile(m_file,
                             b.data(),           
                             b.size(),           
                             &numberOfBytesRead  
        );

    if (!result) {
        
        return -1;
    } else if (numberOfBytesRead == 0) {
        
        return -1;
    }

    return numberOfBytesRead;
}








int ConsoleSaveFileInputStream::read(std::vector<uint8_t>& b,
                                     unsigned int offset, unsigned int length) {
    
    assert(length <= (b.size() - offset));

    unsigned int numberOfBytesRead;

    bool result =
        m_saveFile->readFile(m_file,
                             &b[offset],         
                             length,             
                             &numberOfBytesRead  
        );

    if (!result) {
        
        return -1;
    } else if (numberOfBytesRead == 0) {
        
        return -1;
    }

    return numberOfBytesRead;
}




void ConsoleSaveFileInputStream::close() {
    if (m_saveFile != nullptr) {
        bool result = m_saveFile->closeHandle(m_file);

        if (!result) {
            
        }

        
        m_saveFile = nullptr;
    }
}
