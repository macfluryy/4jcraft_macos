#ifndef WLINUX_H
#define WLINUX_H

#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <cerrno>
#include <atomic>
#include <pthread.h>

#define TRUE true
#define FALSE false
#define RtlZeroMemory(Destination,Length) memset((Destination),0,(Length))
#define ZeroMemory       RtlZeroMemory
#define WINAPI

typedef unsigned int DWORD;
typedef const char *LPCSTR;
typedef bool BOOL;
typedef void* LPVOID;
typedef char WCHAR;
typedef unsigned char BYTE;
typedef BYTE* PBYTE;
typedef const wchar_t* LPCWSTR;
typedef unsigned long long ULONGLONG;
typedef int HRESULT;
typedef unsigned int UINT;
typedef void* HANDLE;
typedef int INT;
typedef long* PLONG;
typedef unsigned int* LPDWORD;
typedef const void* LPCVOID;
typedef char CHAR;
typedef void* PVOID;
typedef unsigned long* ULONG_PTR;
typedef long LONG;
typedef long LONG64, *PLONG64;
typedef void VOID;
typedef ULONGLONG PlayerUID;
typedef DWORD WORD;
typedef struct {
    DWORD LowPart;
    long long QuadPart;
    LONG HighPart;
} LARGE_INTEGER;
typedef long long LONGLONG;
typedef size_t SIZE_T;
typedef std::wstring LPWSTR;
typedef unsigned char boolean; // java brainrot
#define __debugbreak()
#define __int32 int
typedef unsigned long ULONG;
typedef unsigned char byte;

#define PAGE_READWRITE 0x04
#define MEM_LARGE_PAGES 0x20000000
#define MAXULONG_PTR ((ULONG)0xFFFFFFFF)

// https://learn.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-filetime
typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;

typedef struct _MEMORYSTATUS {
	DWORD dwLength;
	DWORD dwMemoryLoad;
	SIZE_T dwTotalPhys;
	SIZE_T dwAvailPhys;
	SIZE_T dwTotalPageFile;
	SIZE_T dwAvailPageFile;
	SIZE_T dwTotalVirtual;
	SIZE_T dwAvailVirtual;
} MEMORYSTATUS, *LPMEMORYSTATUS;

#define FILE_BEGIN SEEK_SET

typedef short SHORT;

typedef VOID* XMEMCOMPRESSION_CONTEXT;
typedef VOID* XMEMDECOMPRESSION_CONTEXT;

typedef float FLOAT;

#define TLS_OUT_OF_INDEXES ((DWORD)0xFFFFFFFF)

typedef pthread_mutex_t RTL_CRITICAL_SECTION;
typedef pthread_mutex_t* PRTL_CRITICAL_SECTION;

typedef RTL_CRITICAL_SECTION CRITICAL_SECTION;
typedef PRTL_CRITICAL_SECTION PCRITICAL_SECTION;
typedef PRTL_CRITICAL_SECTION LPCRITICAL_SECTION;

void InitializeCriticalSection(PRTL_CRITICAL_SECTION CriticalSection) {
    pthread_mutexattr_t attr;
    int ret;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(CriticalSection, &attr);
    pthread_mutexattr_destroy(&attr);
}

void InitializeCriticalSectionAndSpinCount(PRTL_CRITICAL_SECTION CriticalSection, ULONG SpinCount) {
    // no spin count required because we use a recursive mutex
    InitializeCriticalSection(CriticalSection);
}

void DeleteCriticalSection(PRTL_CRITICAL_SECTION CriticalSection) {
    pthread_mutex_destroy(CriticalSection);
}

void EnterCriticalSection(PRTL_CRITICAL_SECTION CriticalSection) {
    pthread_mutex_lock(CriticalSection);
}

void LeaveCriticalSection(PRTL_CRITICAL_SECTION CriticalSection) {
    pthread_mutex_unlock(CriticalSection);
}

ULONG TryEnterCriticalSection(PRTL_CRITICAL_SECTION CriticalSection) {
    return pthread_mutex_trylock(CriticalSection) == 0;
}

// https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-tlsalloc
DWORD TlsAlloc(VOID) {
    pthread_key_t key;

    if (pthread_key_create(&key, NULL) == 0) {
        return key;
    } else {
        return TLS_OUT_OF_INDEXES;
    }
}

// https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-tlsfree
BOOL TlsFree(DWORD dwTlsIndex)
{
    return pthread_key_delete(dwTlsIndex) == 0;
}

// https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-tlsgetvalue
LPVOID TlsGetValue(DWORD dwTlsIndex)
{
    return pthread_getspecific(dwTlsIndex);
}

// https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-tlssetvalue
BOOL TlsSetValue(DWORD dwTlsIndex, LPVOID lpTlsValue)
{
    return pthread_setspecific(dwTlsIndex, lpTlsValue) == 0;
}


// https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-globalmemorystatus
VOID GlobalMemoryStatus(LPMEMORYSTATUS lpBuffer) 
{
    // TODO: Parse /proc/meminfo and set lpBuffer based on that. Probably will also need another
    // different codepath for macOS too.
}

DWORD GetLastError(VOID)
{
    return errno;
}

LONG64 InterlockedCompareExchangeRelease64(
    LONG64 volatile *Destination,
    LONG64 Exchange,
    LONG64 Comperand)
{
    LONG64 expected = Comperand;
    __atomic_compare_exchange_n(Destination, &expected, Exchange, false, __ATOMIC_RELEASE, __ATOMIC_RELAXED);
    return expected;
}



#endif // WLINUX_H
