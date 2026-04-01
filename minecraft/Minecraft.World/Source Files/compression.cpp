#include <assert.h>
#include <string.h>
#include <zconf.h>
#include <cstdint>
#include <mutex>

#include "Minecraft.World/ConsoleHelpers/ConsoleSaveFileIO/FileHeader.h"
#include "Minecraft.Client/Linux/Stubs/winapi_stubs.h"
#include "compression.h"
#include "Minecraft.World/x64headers/extraX64.h"
#if defined(_WIN64) || defined(__linux__)
// zconf.h defines "typedef unsigned char Byte" which conflicts with the
// project's "class Byte" from BasicTypeContainers.h (via stdafx.h).
// Rename zlib's Byte to zlib_Byte before the include so the typedef lands
// under that alias; Bytef (= Byte FAR) will resolve to zlib_Byte as well.
#define Byte zlib_Byte
#include <zlib.h>

#undef Byte
#endif

thread_local Compression::ThreadStorage* Compression::m_tlsCompression =
    nullptr;
Compression::ThreadStorage* Compression::m_tlsCompressionDefault = nullptr;

Compression::ThreadStorage::ThreadStorage() { compression = new Compression(); }

Compression::ThreadStorage::~ThreadStorage() { delete compression; }

void Compression::CreateNewThreadStorage() {
    ThreadStorage* tls = new ThreadStorage();
    if (m_tlsCompressionDefault == nullptr) {
        m_tlsCompressionDefault = tls;
    }
    m_tlsCompression = tls;
}

void Compression::UseDefaultThreadStorage() {
    m_tlsCompression = m_tlsCompressionDefault;
}

void Compression::ReleaseThreadStorage() {
    if (m_tlsCompression != m_tlsCompressionDefault) {
        delete m_tlsCompression;
    }
}

Compression* Compression::getCompression() {
    return m_tlsCompression->compression;
}

int32_t Compression::CompressLZXRLE(void* pDestination, unsigned int* pDestSize,
                                    void* pSource, unsigned int SrcSize) {
    std::lock_guard<std::mutex> lock(rleCompressLock);
    // static unsigned char rleBuf[1024*100];

    unsigned char* pucIn = (unsigned char*)pSource;
    unsigned char* pucEnd = pucIn + SrcSize;
    unsigned char* pucOut = (unsigned char*)rleCompressBuf;

    // Compress with RLE first:
    // 0 - 254 - encodes a single byte
    // 255 followed by 0, 1, 2 - encodes a 1, 2, or 3 255s
    // 255 followed by 3-255, followed by a byte - encodes a run of n + 1 bytes
    PIXBeginNamedEvent(0, "RLE compression");
    do {
        unsigned char thisOne = *pucIn++;

        unsigned int count = 1;
        while ((pucIn != pucEnd) && (*pucIn == thisOne) && (count < 256)) {
            pucIn++;
            count++;
        }

        if (count <= 3) {
            if (thisOne == 255) {
                *pucOut++ = 255;
                *pucOut++ = count - 1;
            } else {
                for (unsigned int i = 0; i < count; i++) {
                    *pucOut++ = thisOne;
                }
            }
        } else {
            *pucOut++ = 255;
            *pucOut++ = count - 1;
            *pucOut++ = thisOne;
        }
    } while (pucIn != pucEnd);
    unsigned int rleSize = (unsigned int)(pucOut - rleCompressBuf);
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0, "Secondary compression");
    Compress(pDestination, pDestSize, rleCompressBuf, rleSize);
    PIXEndNamedEvent();
    //	printf("Compressed from %d to %d to %d\n",SrcSize,rleSize,*pDestSize);

    return 0;
}

int32_t Compression::CompressRLE(void* pDestination, unsigned int* pDestSize,
                                 void* pSource, unsigned int SrcSize) {
    unsigned int rleSize;
    {
        std::lock_guard<std::mutex> lock(rleCompressLock);
        // static unsigned char rleBuf[1024*100];

        unsigned char* pucIn = (unsigned char*)pSource;
        unsigned char* pucEnd = pucIn + SrcSize;
        unsigned char* pucOut = (unsigned char*)rleCompressBuf;

        // Compress with RLE first:
        // 0 - 254 - encodes a single byte
        // 255 followed by 0, 1, 2 - encodes a 1, 2, or 3 255s
        // 255 followed by 3-255, followed by a byte - encodes a run of n + 1
        // bytes
        PIXBeginNamedEvent(0, "RLE compression");
        do {
            unsigned char thisOne = *pucIn++;

            unsigned int count = 1;
            while ((pucIn != pucEnd) && (*pucIn == thisOne) && (count < 256)) {
                pucIn++;
                count++;
            }

            if (count <= 3) {
                if (thisOne == 255) {
                    *pucOut++ = 255;
                    *pucOut++ = count - 1;
                } else {
                    for (unsigned int i = 0; i < count; i++) {
                        *pucOut++ = thisOne;
                    }
                }
            } else {
                *pucOut++ = 255;
                *pucOut++ = count - 1;
                *pucOut++ = thisOne;
            }
        } while (pucIn != pucEnd);
        rleSize = (unsigned int)(pucOut - rleCompressBuf);
        PIXEndNamedEvent();
    }

    // Return
    if (rleSize <= *pDestSize) {
        *pDestSize = rleSize;
        memcpy(pDestination, rleCompressBuf, *pDestSize);
    } else {
#if !defined(_CONTENT_PACKAGE)
        assert(false);
#endif
    }

    return 0;
}

int32_t Compression::DecompressLZXRLE(void* pDestination,
                                      unsigned int* pDestSize, void* pSource,
                                      unsigned int SrcSize) {
    std::lock_guard<std::mutex> lock(rleDecompressLock);
    // 4J Stu - Fix for #13676 - Crash: Crash while attempting to load a world
    // after updating TU Some saves can have chunks that decompress into very
    // large sizes, so I have doubled the size of this buffer Ideally we should
    // be able to dynamically allocate the buffer if it's going to be too big,
    // as most chunks only use 5% of this buffer

    // 4J Stu - Changed this again to dynamically allocate a buffer if it's
    // going to be too big
    unsigned char* pucIn = nullptr;

    // const unsigned int staticRleSize = 1024*200;
    // static unsigned char rleBuf[staticRleSize];
    unsigned int rleSize = staticRleSize;
    unsigned char* dynamicRleBuf = nullptr;

    if (*pDestSize > rleSize) {
        rleSize = *pDestSize;
        dynamicRleBuf = new unsigned char[rleSize];
        Decompress(dynamicRleBuf, &rleSize, pSource, SrcSize);
        pucIn = (unsigned char*)dynamicRleBuf;
    } else {
        Decompress(rleDecompressBuf, &rleSize, pSource, SrcSize);
        pucIn = (unsigned char*)rleDecompressBuf;
    }

    // unsigned char *pucIn = (unsigned char *)rleDecompressBuf;
    unsigned char* pucEnd = pucIn + rleSize;
    unsigned char* pucOut = (unsigned char*)pDestination;

    while (pucIn != pucEnd) {
        unsigned char thisOne = *pucIn++;
        if (thisOne == 255) {
            unsigned int count = *pucIn++;
            if (count < 3) {
                count++;
                for (unsigned int i = 0; i < count; i++) {
                    *pucOut++ = 255;
                }
            } else {
                count++;
                unsigned char data = *pucIn++;
                for (unsigned int i = 0; i < count; i++) {
                    *pucOut++ = data;
                }
            }
        } else {
            *pucOut++ = thisOne;
        }
    }
    *pDestSize = (unsigned int)(pucOut - (unsigned char*)pDestination);

    //	printf("Decompressed from %d to %d to %d\n",SrcSize,rleSize,*pDestSize);

    if (dynamicRleBuf != nullptr) delete[] dynamicRleBuf;

    return 0;
}

int32_t Compression::DecompressRLE(void* pDestination, unsigned int* pDestSize,
                                   void* pSource, unsigned int SrcSize) {
    std::lock_guard<std::mutex> lock(rleDecompressLock);

    // unsigned char *pucIn = (unsigned char *)rleDecompressBuf;
    unsigned char* pucIn = (unsigned char*)pSource;
    unsigned char* pucEnd = pucIn + SrcSize;
    unsigned char* pucOut = (unsigned char*)pDestination;

    while (pucIn != pucEnd) {
        unsigned char thisOne = *pucIn++;
        if (thisOne == 255) {
            unsigned int count = *pucIn++;
            if (count < 3) {
                count++;
                for (unsigned int i = 0; i < count; i++) {
                    *pucOut++ = 255;
                }
            } else {
                count++;
                unsigned char data = *pucIn++;
                for (unsigned int i = 0; i < count; i++) {
                    *pucOut++ = data;
                }
            }
        } else {
            *pucOut++ = thisOne;
        }
    }
    *pDestSize = (unsigned int)(pucOut - (unsigned char*)pDestination);

    return 0;
}

int32_t Compression::Compress(void* pDestination, unsigned int* pDestSize,
                              void* pSource, unsigned int SrcSize) {
    // Using zlib for x64 compression - 360 is using native 360 compression and
    // PS3 a stubbed non-compressing version of this
#if defined(_WIN64) || defined(__linux__)
    size_t destSize = (size_t)(*pDestSize);
    int res = ::compress((Bytef*)pDestination, (uLongf*)&destSize,
                         (Bytef*)pSource, SrcSize);
    *pDestSize = (unsigned int)destSize;
    return ((res == Z_OK) ? 0 : -1);
#else
    size_t destSize = (size_t)(*pDestSize);
    int32_t res = XMemCompress(compressionContext, pDestination, &destSize,
                               pSource, SrcSize);
    *pDestSize = (unsigned int)destSize;
    return res;
#endif
}

int32_t Compression::Decompress(void* pDestination, unsigned int* pDestSize,
                                void* pSource, unsigned int SrcSize) {
    if (m_decompressType !=
        m_localDecompressType)  // check if we're decompressing data from a
                                // different platform
    {
        // only used for loading a save from a different platform (Sony cloud
        // storage cross play)
        return DecompressWithType(pDestination, pDestSize, pSource, SrcSize);
    }

    // Using zlib for x64 compression - 360 is using native 360 compression and
    // PS3 a stubbed non-compressing version of this
#if defined(_WIN64) || defined(__linux__)
    size_t destSize = (size_t)(*pDestSize);
    int res = ::uncompress((Bytef*)pDestination, (uLongf*)&destSize,
                           (Bytef*)pSource, SrcSize);
    *pDestSize = (unsigned int)destSize;
    return ((res == Z_OK) ? 0 : -1);
#else
    size_t destSize = (size_t)(*pDestSize);
    int32_t res = XMemDecompress(decompressionContext, pDestination,
                                 (size_t*)&destSize, pSource, SrcSize);
    *pDestSize = (unsigned int)destSize;
    return res;
#endif
}

// MGH -  same as VirtualDecompress in PSVitaStubs, but for use on other
// platforms (so no virtual mem stuff)
void Compression::VitaVirtualDecompress(
    void* pDestination, unsigned int* pDestSize, void* pSource,
    unsigned int SrcSize)  // (void* buf, size_t dwSize, void* dst)
{
    std::uint8_t* pSrc = (std::uint8_t*)pSource;
    int Offset = 0;
    int Page = 0;
    int Index = 0;
    std::uint8_t* Data = (std::uint8_t*)pDestination;
    while (Index != SrcSize) {
        // is this a normal value
        if (pSrc[Index]) {
            // just copy it across
            Data[Offset] = pSrc[Index];
            Offset += 1;
        } else {
            // how many zeros do we have
            Index += 1;
            int Count = pSrc[Index];
            // to do : this should really be a sequence of memsets
            for (int i = 0; i < Count; i += 1) {
                Data[Offset] = 0;
                Offset += 1;
            }
        }
        Index += 1;
    }
    *pDestSize = Offset;
}

int32_t Compression::DecompressWithType(void* pDestination,
                                        unsigned int* pDestSize, void* pSource,
                                        unsigned int SrcSize) {
    switch (m_decompressType) {
        case eCompressionType_RLE:  // 4J-JEV, RLE is just that; don't want to
                                    // break here though.
        case eCompressionType_None:
            memcpy(pDestination, pSource, SrcSize);
            *pDestSize = SrcSize;
            return 0;
        case eCompressionType_LZXRLE: {
#if defined(_WIN64)
            size_t destSize = (size_t)(*pDestSize);
            int32_t res = XMemDecompress(decompressionContext, pDestination,
                                         (size_t*)&destSize, pSource, SrcSize);
            *pDestSize = (unsigned int)destSize;
            return res;
#else
            assert(0);
#endif
        } break;
        case eCompressionType_ZLIBRLE:
#if defined(_WIN64) || defined(__linux__)
            if (pDestination != nullptr)
                return ::uncompress(
                    (Bytef*)pDestination, (unsigned long*)pDestSize,
                    (const Bytef*)pSource, SrcSize);  // Decompress
            else
                break;  // Cannot decompress when destination is nullptr
#else
            assert(0);
            break;
#endif
        case eCompressionType_PS3ZLIB:
#if defined(_WIN64)
            // Note that we're missing the normal zlib header and footer so
            // we'll use inflate to decompress the payload and skip all the CRC
            // checking, etc
            if (pDestination != nullptr) {
                // Read big-endian srcize from array
                std::uint8_t* pbDestSize =
                    reinterpret_cast<std::uint8_t*>(pDestSize);
                std::uint8_t* pbSource =
                    reinterpret_cast<std::uint8_t*>(pSource);
                for (int i = 3; i >= 0; i--) {
                    pbDestSize[3 - i] = pbSource[i];
                }

                std::vector<uint8_t> uncompr = std::vector<uint8_t>(*pDestSize);

                // Build decompression stream
                z_stream strm;
                strm.zalloc = Z_NULL;
                strm.zfree = Z_NULL;
                strm.opaque = Z_NULL;
                strm.next_out = uncompr.data;
                strm.avail_out = uncompr.length;
                // Skip those first 4 bytes
                strm.next_in = reinterpret_cast<std::uint8_t*>(pSource) + 4;
                strm.avail_in = SrcSize - 4;

                int hr = inflateInit2(&strm, -15);

                // Run inflate() on input until end of stream
                do {
                    hr = inflate(&strm, Z_NO_FLUSH);

                    // Check
                    switch (hr) {
                        case Z_NEED_DICT:
                        case Z_DATA_ERROR:
                        case Z_MEM_ERROR:
                        case Z_STREAM_ERROR:
                            (void)inflateEnd(&strm);
                            assert(false);
                    }
                } while (hr != Z_STREAM_END);

                inflateEnd(
                    &strm);  // MGH - added, to clean up zlib, was causing a
                             // leak on Vita when dowloading a PS3 save

                // Copy the uncompressed data to the destination
                memcpy(pDestination, uncompr.data, uncompr.length);
                *pDestSize = uncompr.length;

                // Delete uncompressed data
                delete uncompr.data;
                return 0;
            } else
                break;  // Cannot decompress when destination is nullptr
#else
            assert(0);
#endif
    }

    assert(false);
    return -1;
}

Compression::Compression() {
    // Using zlib for x64 compression - 360 is using native 360 compression and
    // PS3 a stubbed non-compressing version of this
    // The default parameters for compression context allocated about 6.5MB,
    // reducing the partition size here from the default 512KB to 128KB brings
    // this down to about 3MB
    XMEMCODEC_PARAMETERS_LZX params;
    params.Flags = 0;
    params.WindowSize = 128 * 1024;
    params.CompressionPartitionSize = 128 * 1024;

    XMemCreateCompressionContext(XMEMCODEC_LZX, &params, 0,
                                 &compressionContext);
    XMemCreateDecompressionContext(XMEMCODEC_LZX, &params, 0,
                                   &decompressionContext);

    m_localDecompressType = eCompressionType_ZLIBRLE;
    m_decompressType = m_localDecompressType;
}

Compression::~Compression() {
    XMemDestroyCompressionContext(compressionContext);
    XMemDestroyDecompressionContext(decompressionContext);
}

void Compression::SetDecompressionType(ESavePlatform platform) {
    switch (platform) {
        case SAVE_FILE_PLATFORM_X360:
            Compression::getCompression()->SetDecompressionType(
                Compression::eCompressionType_LZXRLE);
            break;
        case SAVE_FILE_PLATFORM_PS3:
            Compression::getCompression()->SetDecompressionType(
                Compression::eCompressionType_PS3ZLIB);
            break;
        case SAVE_FILE_PLATFORM_XBONE:
        case SAVE_FILE_PLATFORM_PS4:
        case SAVE_FILE_PLATFORM_PSVITA:
        case SAVE_FILE_PLATFORM_WIN64:
            Compression::getCompression()->SetDecompressionType(
                Compression::eCompressionType_ZLIBRLE);
            break;
        default:
            assert(0);
            break;
    }
}

/*Compression gCompression;*/
