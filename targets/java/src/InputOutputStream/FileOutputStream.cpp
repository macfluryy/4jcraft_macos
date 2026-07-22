#include "java/InputOutputStream/FileOutputStream.h"

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

#include <filesystem>

#include "java/File.h"













FileOutputStream::FileOutputStream(const File& file) : m_fileHandle(nullptr) {
    if (file.exists() && file.isDirectory()) {
        
        return;
    }

#if defined(_WIN32)
    m_fileHandle = _wfopen(file.getPath().c_str(), L"wb");
#else
    const std::string nativePath = std::filesystem::path(file.getPath()).string();
    m_fileHandle = std::fopen(nativePath.c_str(), "wb");
#endif

    if (m_fileHandle == nullptr) {
        
        perror("FileOutputStream::FileOutputStream");
    }
}

FileOutputStream::~FileOutputStream() {
    if (m_fileHandle != nullptr) {
        std::fclose(m_fileHandle);
    }
}



void FileOutputStream::write(unsigned int b) {
    if (m_fileHandle == nullptr) {
        return;
    }

    std::uint8_t value = (std::uint8_t)b;
    const size_t numberOfBytesWritten = std::fwrite(&value, 1, 1, m_fileHandle);
    const int result = std::ferror(m_fileHandle);

    if (result != 0) {
        
    } else if (numberOfBytesWritten == 0) {
        
    }
}



void FileOutputStream::write(const std::vector<uint8_t>& b) {
    if (m_fileHandle == nullptr) {
        return;
    }

    const size_t numberOfBytesWritten =
        std::fwrite(b.data(), 1, b.size(), m_fileHandle);
    const int result = std::ferror(m_fileHandle);

    if (result != 0) {
        
    } else if (numberOfBytesWritten == 0 || numberOfBytesWritten != b.size()) {
        
    }
}




void FileOutputStream::write(const std::vector<uint8_t>& b, unsigned int offset,
                             unsigned int length) {
    
    assert(length <= (b.size() - offset));

    if (m_fileHandle == nullptr) {
        return;
    }

    const size_t numberOfBytesWritten =
        std::fwrite(&b[offset], 1, length, m_fileHandle);
    const int result = std::ferror(m_fileHandle);

    if (result != 0) {
        
    } else if (numberOfBytesWritten == 0 || numberOfBytesWritten != length) {
        
    }
}





void FileOutputStream::close() {
    if (m_fileHandle == nullptr) {
        return;
    }

    int result = std::fclose(m_fileHandle);
    if (result != 0) {
        
    }

    
    m_fileHandle = nullptr;
}

void FileOutputStream::flush() {
    if (m_fileHandle != nullptr) {
        std::fflush(m_fileHandle);
    }
}
