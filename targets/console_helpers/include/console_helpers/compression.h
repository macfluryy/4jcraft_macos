#pragma once
#include <mutex>

#include "minecraft/world/level/storage/ConsoleSaveFileIO/FileHeader.h"

class Compression {
public:
    // Enum maps directly some external tools
    enum ECompressionTypes {
        eCompressionType_None = 0,
        eCompressionType_RLE = 1,
        eCompressionType_LZXRLE = 2,
        eCompressionType_ZLIBRLE = 3,
        eCompressionType_PS3ZLIB = 4
    };

private:
    // 4J added so we can have separate contexts and rleBuf for different
    // threads
    class ThreadStorage {
    public:
        Compression* compression;
        ThreadStorage();
        ~ThreadStorage();
    };
    static thread_local ThreadStorage* m_tlsCompression;
    static ThreadStorage* m_tlsCompressionDefault;

public:
    // Each new thread that needs to use Compression will need to call one of
    // the following 2 functions, to either create its own local storage, or
    // share the default storage already allocated by the main thread
    static void CreateNewThreadStorage();
    static void UseDefaultThreadStorage();
    static void ReleaseThreadStorage();

    static Compression* getCompression();

public:
    int32_t Compress(void* pDestination, unsigned int* pDestSize, void* pSource,
                     unsigned int SrcSize);
    int32_t Decompress(void* pDestination, unsigned int* pDestSize,
                       void* pSource, unsigned int SrcSize);
    int32_t CompressLZXRLE(void* pDestination, unsigned int* pDestSize,
                           void* pSource, unsigned int SrcSize);
    int32_t DecompressLZXRLE(void* pDestination, unsigned int* pDestSize,
                             void* pSource, unsigned int SrcSize);
    int32_t CompressRLE(void* pDestination, unsigned int* pDestSize,
                        void* pSource, unsigned int SrcSize);
    int32_t DecompressRLE(void* pDestination, unsigned int* pDestSize,
                          void* pSource, unsigned int SrcSize);
    static void VitaVirtualDecompress(void* pDestination,
                                      unsigned int* pDestSize, void* pSource,
                                      unsigned int SrcSize);

    void SetDecompressionType(ECompressionTypes type) {
        m_decompressType = type;
    }  // for loading a save from a different platform (Sony cloud storage cross
       // play)
    ECompressionTypes GetDecompressionType() { return m_decompressType; }
    void SetDecompressionType(ESavePlatform platform);

    Compression();
    ~Compression();

private:
    int32_t DecompressWithType(void* pDestination, unsigned int* pDestSize,
                               void* pSource, unsigned int SrcSize);

    void* compressionContext;
    void* decompressionContext;
    std::mutex rleCompressLock;
    std::mutex rleDecompressLock;

    unsigned char rleCompressBuf[1024 * 100];
    static const unsigned int staticRleSize = 1024 * 200;
    unsigned char rleDecompressBuf[staticRleSize];
    ECompressionTypes m_decompressType;
    ECompressionTypes m_localDecompressType;
};

// extern Compression gCompression;

#if defined(_WIN64) || defined(__linux__)
#define APPROPRIATE_COMPRESSION_TYPE Compression::eCompressionType_ZLIBRLE
#else
#define APPROPRIATE_COMPRESSION_TYPE Compression::eCompressionType_LZXRLE
#endif
