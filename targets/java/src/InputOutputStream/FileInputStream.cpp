
#include "java/InputOutputStream/FileInputStream.h"

#include <assert.h>
#include <sys/types.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include <filesystem>

#include "java/File.h"

namespace {
int64_t FileTell(std::FILE* file) {
#if defined(_WIN32)
    return _ftelli64(file);
#else
    return static_cast<int64_t>(ftello(file));
#endif
}

bool FileSeek(std::FILE* file, int64_t offset, int origin) {
#if defined(_WIN32)
    return _fseeki64(file, offset, origin) == 0;
#else
    return fseeko(file, static_cast<off_t>(offset), origin) == 0;
#endif
}
}  


















FileInputStream::FileInputStream(const File& file) : m_fileHandle(nullptr) {
#if defined(_WIN32)
    m_fileHandle = _wfopen(file.getPath().c_str(), L"rb");
#else
    const std::string nativePath = std::filesystem::path(file.getPath()).string();
    m_fileHandle = std::fopen(nativePath.c_str(), "rb");
#endif

    if (m_fileHandle == nullptr) {
        assert(0);
    }
}

FileInputStream::~FileInputStream() {
    if (m_fileHandle != nullptr) {
        std::fclose(m_fileHandle);
    }
}




int FileInputStream::read() {
    if (m_fileHandle == nullptr) {
        return -1;
    }

    std::uint8_t byteRead = static_cast<std::uint8_t>(0);
    const size_t numberOfBytesRead = std::fread(&byteRead, 1, 1, m_fileHandle);

    if (std::ferror(m_fileHandle) != 0) {
        assert(0);
    } else if (numberOfBytesRead == 0) {
        
        return -1;
    }

    return static_cast<int>(byteRead);
}






int FileInputStream::read(std::vector<uint8_t>& b) {
    if (m_fileHandle == nullptr) {
        return -1;
    }

    const size_t numberOfBytesRead =
        std::fread(b.data(), 1, b.size(), m_fileHandle);

    if (std::ferror(m_fileHandle) != 0) {
        assert(0);
    } else if (numberOfBytesRead == 0) {
        
        return -1;
    }

    return numberOfBytesRead;
}








int FileInputStream::read(std::vector<uint8_t>& b, unsigned int offset,
                          unsigned int length) {
    
    assert(length <= (b.size() - offset));

    if (m_fileHandle == nullptr) {
        return -1;
    }

    const size_t numberOfBytesRead =
        std::fread(&b[offset], 1, length, m_fileHandle);

    if (std::ferror(m_fileHandle) != 0) {
        assert(0);
    } else if (numberOfBytesRead == 0) {
        
        return -1;
    }

    return numberOfBytesRead;
}




void FileInputStream::close() {
    if (m_fileHandle == nullptr) {
        
        
        return;
    }

    int result = std::fclose(m_fileHandle);

    if (result != 0) {
        
    }

    
    m_fileHandle = nullptr;
}







int64_t FileInputStream::skip(int64_t n) {
    if (m_fileHandle == nullptr || n <= 0) {
        return 0;
    }

    const int64_t start = FileTell(m_fileHandle);
    if (start < 0) {
        return 0;
    }

    if (!FileSeek(m_fileHandle, 0, SEEK_END)) {
        return 0;
    }

    const int64_t end = FileTell(m_fileHandle);
    if (end < 0) {
        return 0;
    }

    const int64_t offset = std::min(n, std::max<int64_t>(0, end - start));
    const int64_t target = start + offset;
    if (!FileSeek(m_fileHandle, target, SEEK_SET)) {
        return 0;
    }

    return offset;
}
