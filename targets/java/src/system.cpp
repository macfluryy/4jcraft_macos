#include "java/System.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <algorithm>
#include <chrono>
#include <format>
#include <vector>

class Biome;
class Node;

template <class T>
void System::arraycopy(const std::vector<T>& src, unsigned int srcPos,
                       std::vector<T>* dst, unsigned int dstPos,
                       unsigned int length) {
    assert(srcPos >= 0 && srcPos <= src.size());
    assert(srcPos + length <= src.size());
    assert(dstPos + length <= dst->size());

    std::copy(src.data() + srcPos, src.data() + srcPos + length,
              dst->data() + dstPos);
}

ArrayCopyFunctionDefinition(Node*) ArrayCopyFunctionDefinition(Biome*)

    void System::arraycopy(const std::vector<uint8_t>& src, unsigned int srcPos,
                           std::vector<uint8_t>* dst, unsigned int dstPos,
                           unsigned int length) {
    assert(srcPos >= 0 && srcPos <= src.size());
    assert(srcPos + length <= src.size());
    assert(dstPos + length <= dst->size());

    memcpy(dst->data() + dstPos, src.data() + srcPos, length);
}

void System::arraycopy(const std::vector<int>& src, unsigned int srcPos,
                       std::vector<int>* dst, unsigned int dstPos,
                       unsigned int length) {
    assert(srcPos >= 0 && srcPos <= src.size());
    assert(srcPos + length <= src.size());
    assert(dstPos + length <= dst->size());

    memcpy(dst->data() + dstPos, src.data() + srcPos, length * sizeof(int));
}





















int64_t System::nanoTime() {
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
}











int64_t System::currentTimeMillis() {
    auto now = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
}



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
