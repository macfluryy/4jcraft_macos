#include "minecraft/world/level/storage/ConsoleSaveFileIO/ConsoleSaveFileOutputStream.h"

#include <assert.h>

#include <cstdint>
#include <vector>

#include "minecraft/world/level/storage/ConsoleSaveFileIO/ConsoleSaveFile.h"

class ConsoleSavePath;













ConsoleSaveFileOutputStream::ConsoleSaveFileOutputStream(
    ConsoleSaveFile* saveFile, const ConsoleSavePath& file) {
    m_saveFile = saveFile;

    m_file = m_saveFile->createFile(file);

    m_saveFile->setFilePointer(m_file, 0, SaveFileSeekOrigin::Begin);
}

ConsoleSaveFileOutputStream::ConsoleSaveFileOutputStream(
    ConsoleSaveFile* saveFile, FileEntry* file) {
    m_saveFile = saveFile;

    m_file = file;

    m_saveFile->setFilePointer(m_file, 0, SaveFileSeekOrigin::Begin);
}



void ConsoleSaveFileOutputStream::write(unsigned int b) {
    unsigned int numberOfBytesWritten;

    std::uint8_t value = (std::uint8_t)b;

    bool result =
        m_saveFile->writeFile(m_file,
                              &value,                
                              1,                     
                              &numberOfBytesWritten  
        );

    if (!result) {
        
    } else if (numberOfBytesWritten == 0) {
        
    }
}



void ConsoleSaveFileOutputStream::write(const std::vector<uint8_t>& b) {
    unsigned int numberOfBytesWritten;

    bool result =
        m_saveFile->writeFile(m_file,
                              b.data(),              
                              b.size(),              
                              &numberOfBytesWritten  
        );

    if (!result) {
        
    } else if (numberOfBytesWritten == 0 || numberOfBytesWritten != b.size()) {
        
    }
}




void ConsoleSaveFileOutputStream::write(const std::vector<uint8_t>& b,
                                        unsigned int offset,
                                        unsigned int length) {
    
    assert(length <= (b.size() - offset));

    unsigned int numberOfBytesWritten;

    bool result =
        m_saveFile->writeFile(m_file,
                              &b[offset],            
                              length,                
                              &numberOfBytesWritten  
        );

    if (!result) {
        
    } else if (numberOfBytesWritten == 0 || numberOfBytesWritten != length) {
        
    }
}





void ConsoleSaveFileOutputStream::close() {
    if (m_saveFile != nullptr) {
        bool result = m_saveFile->closeHandle(m_file);

        if (!result) {
            
        }

        
        m_saveFile = nullptr;
    }
}
