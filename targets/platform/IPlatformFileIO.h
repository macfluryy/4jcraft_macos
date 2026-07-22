#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <vector>


class IPlatformFileIO {
public:
    enum class ReadStatus {
        Ok,
        NotFound,
        TooLarge,
        ReadError,
    };

    struct ReadResult {
        ReadStatus status;
        std::size_t bytesRead;
        std::size_t fileSize;
    };

    virtual ~IPlatformFileIO() = default;

    
    [[nodiscard]] virtual ReadResult readFile(
        const std::filesystem::path& path, void* buffer,
        std::size_t capacity) = 0;

    
    [[nodiscard]] virtual ReadResult readFileSegment(
        const std::filesystem::path& path, std::size_t offset, void* buffer,
        std::size_t bytesToRead) = 0;

    
    [[nodiscard]] virtual std::vector<std::uint8_t> readFileToVec(
        const std::filesystem::path& path) = 0;

    
    virtual bool writeFile(const std::filesystem::path& path,
                           const void* buffer, std::size_t bytesToWrite) = 0;

    
    [[nodiscard]] virtual bool exists(const std::filesystem::path& path) = 0;

    
    [[nodiscard]] virtual std::size_t fileSize(
        const std::filesystem::path& path) = 0;

    
    [[nodiscard]] virtual std::filesystem::path getBasePath() = 0;

    
    [[nodiscard]] virtual std::filesystem::path getUserDataPath() = 0;
};
