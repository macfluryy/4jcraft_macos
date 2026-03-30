#include "../Header Files/stdafx.h"
#include "System.h"
#if defined(__linux__)
#include <sys/time.h>
#include <time.h>
#include <ctime>
#endif

template <class T>
void System::arraycopy(arrayWithLength<T> src, unsigned int srcPos,
                       arrayWithLength<T>* dst, unsigned int dstPos,
                       unsigned int length) {
    assert(srcPos >= 0 && srcPos <= src.length);
    assert(srcPos + length <= src.length);
    assert(dstPos + length <= dst->length);

    std::copy(src.data + srcPos, src.data + srcPos + length,
              dst->data + dstPos);
}

ArrayCopyFunctionDefinition(Node*) ArrayCopyFunctionDefinition(Biome*)

    void System::arraycopy(arrayWithLength<uint8_t> src, unsigned int srcPos,
                           arrayWithLength<uint8_t>* dst, unsigned int dstPos,
                           unsigned int length) {
    assert(srcPos >= 0 && srcPos <= src.length);
    assert(srcPos + length <= src.length);
    assert(dstPos + length <= dst->length);

    memcpy(dst->data + dstPos, src.data + srcPos, length);
}

void System::arraycopy(arrayWithLength<int> src, unsigned int srcPos,
                       arrayWithLength<int>* dst, unsigned int dstPos,
                       unsigned int length) {
    assert(srcPos >= 0 && srcPos <= src.length);
    assert(srcPos + length <= src.length);
    assert(dstPos + length <= dst->length);

    memcpy(dst->data + dstPos, src.data + srcPos, length * sizeof(int));
}

// TODO 4J Stu - These time functions may suffer from accuracy and we might have
// to use a high-resolution timer
// Returns the current value of the most precise available system timer, in
// nanoseconds. This method can only be used to measure elapsed time and is not
// related to any other notion of system or wall-clock time. The value returned
// represents nanoseconds since some fixed but arbitrary time (perhaps in the
// future, so values may be negative). This method provides nanosecond
// precision, but not necessarily nanosecond accuracy. No guarantees are made
// about how frequently values change. Differences in successive calls that span
// greater than approximately 292 years (263 nanoseconds) will not accurately
// compute elapsed time due to numerical overflow.
//
// For example, to measure how long some code takes to execute:
//
//   long startTime = System.nanoTime();
//   // ... the code being measured ...
//   long estimatedTime = System.nanoTime() - startTime;
//
// Returns:
// The current value of the system timer, in nanoseconds.
int64_t System::nanoTime() {
#if !defined(__linux__)
    return GetTickCount() * 1000000LL;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<int64_t>(ts.tv_sec) * 1000000000LL + ts.tv_nsec;
#endif
}

// Returns the current time in milliseconds. Note that while the unit of time of
// the return value is a millisecond, the granularity of the value depends on
// the underlying operating system and may be larger. For example, many
// operating systems measure time in units of tens of milliseconds. See the
// description of the class Date for a discussion of slight discrepancies that
// may arise between "computer time" and coordinated universal time (UTC).
//
// Returns:
// the difference, measured in milliseconds, between the current time and
// midnight, January 1, 1970 UTC.
int64_t System::currentTimeMillis() {
#if defined(__linux__)
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    // Convert to milliseconds since unix epoch instead of windows file time
    // time is expecting calculation to be between 10-30 ms.
    return (int64_t)tv.tv_sec * 1000LL + tv.tv_usec / 1000;
#else

    SYSTEMTIME UTCSysTime;
    GetSystemTime(&UTCSysTime);

    // Represents as a 64-bit value the number of 100-nanosecond intervals since
    // January 1, 1601
    FILETIME UTCFileTime;
    SystemTimeToFileTime(&UTCSysTime, &UTCFileTime);

    LARGE_INTEGER li;
    li.HighPart = UTCFileTime.dwHighDateTime;
    li.LowPart = UTCFileTime.dwLowDateTime;

    return li.QuadPart / 10000;
#endif
}

// 4J Stu - Added this so that we can use real-world timestamps in PSVita saves.
// Particularly required for the save transfers to be smooth
int64_t System::currentRealTimeMillis() { return currentTimeMillis(); }

void System::ReverseUSHORT(unsigned short* pusVal) {
    unsigned short usValue = *pusVal;
    unsigned char* pchVal1 = (unsigned char*)pusVal;
    unsigned char* pchVal2 = (unsigned char*)&usValue;

    pchVal1[0] = pchVal2[1];
    pchVal1[1] = pchVal2[0];
}

void System::ReverseSHORT(short* pusVal) {
    short usValue = *pusVal;
    unsigned char* pchVal1 = (unsigned char*)pusVal;
    unsigned char* pchVal2 = (unsigned char*)&usValue;

    pchVal1[0] = pchVal2[1];
    pchVal1[1] = pchVal2[0];
}

void System::ReverseULONG(unsigned long* pulVal) {
    unsigned long ulValue = *pulVal;
    unsigned char* pchVal1 = (unsigned char*)pulVal;
    unsigned char* pchVal2 = (unsigned char*)&ulValue;

    pchVal1[0] = pchVal2[3];
    pchVal1[1] = pchVal2[2];
    pchVal1[2] = pchVal2[1];
    pchVal1[3] = pchVal2[0];
}

void System::ReverseULONG(unsigned int* pulVal) {
    unsigned int ulValue = *pulVal;
    unsigned char* pchVal1 = (unsigned char*)pulVal;
    unsigned char* pchVal2 = (unsigned char*)&ulValue;

    pchVal1[0] = pchVal2[3];
    pchVal1[1] = pchVal2[2];
    pchVal1[2] = pchVal2[1];
    pchVal1[3] = pchVal2[0];
}

void System::ReverseINT(int* piVal) {
    int ulValue = *piVal;
    unsigned char* pchVal1 = (unsigned char*)piVal;
    unsigned char* pchVal2 = (unsigned char*)&ulValue;

    pchVal1[0] = pchVal2[3];
    pchVal1[1] = pchVal2[2];
    pchVal1[2] = pchVal2[1];
    pchVal1[3] = pchVal2[0];
}

void System::ReverseULONGLONG(int64_t* pullVal) {
    int64_t ullValue = *pullVal;
    unsigned char* pchVal1 = (unsigned char*)pullVal;
    unsigned char* pchVal2 = (unsigned char*)&ullValue;

    pchVal1[0] = pchVal2[7];
    pchVal1[1] = pchVal2[6];
    pchVal1[2] = pchVal2[5];
    pchVal1[3] = pchVal2[4];
    pchVal1[4] = pchVal2[3];
    pchVal1[5] = pchVal2[2];
    pchVal1[6] = pchVal2[1];
    pchVal1[7] = pchVal2[0];
}

void System::ReverseWCHARA(wchar_t* pwch, int iLen) {
    for (int i = 0; i < iLen; i++) {
        ReverseUSHORT((unsigned short*)&pwch[i]);
    }
}
