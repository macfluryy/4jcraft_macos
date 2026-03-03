#ifndef LINUXSTUBS_H
#define LINUXSTUBS_H

#pragma once

#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <string>
#include <cerrno>
#include <atomic>
#include <climits>
#include <cfloat>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fnmatch.h>
#include <time.h>
#include <stdio.h>

#define TRUE true
#define FALSE false
#define RtlZeroMemory(Destination,Length) memset((Destination),0,(Length))
#define ZeroMemory RtlZeroMemory
#define WINAPI

typedef unsigned int DWORD;
typedef const char *LPCSTR;
typedef bool BOOL;
typedef BOOL *PBOOL;
typedef BOOL *LPBOOL;
typedef void* LPVOID;
typedef wchar_t WCHAR;
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
typedef uintptr_t ULONG_PTR;
typedef long LONG;
typedef long LONG64, *PLONG64;
typedef void VOID;
typedef ULONGLONG PlayerUID;
typedef DWORD WORD;
typedef DWORD* PDWORD;
typedef struct {
    DWORD LowPart;
    LONG HighPart;
    long long QuadPart;
} LARGE_INTEGER;
typedef long long LONGLONG;
typedef size_t SIZE_T;
typedef std::wstring LPWSTR;
typedef unsigned char boolean; // java brainrot
#define __debugbreak()
#define __int32 int
typedef int64_t __int64;
typedef uint64_t __uint64;
typedef unsigned long ULONG;
// typedef unsigned char byte;
typedef short SHORT;
typedef float FLOAT;

#define ERROR_SUCCESS                    0L
#define ERROR_IO_PENDING                 997L    // dderror
#define ERROR_CANCELLED                  1223L
//#define S_OK                                   ((HRESULT)0x00000000L)
#define S_FALSE                                ((HRESULT)0x00000001L)

#define PAGE_READWRITE 0x04
#define MEM_LARGE_PAGES 0x20000000
#define MAXULONG_PTR ((ULONG_PTR)~0UL)
#define MAX_PATH 260

#define GENERIC_READ 0x80000000UL
#define GENERIC_WRITE 0x40000000UL
#define GENERIC_EXECUTE 0x20000000UL
#define GENERIC_ALL 0x10000000UL

#define FILE_SHARE_READ 0x00000001
#define FILE_SHARE_WRITE 0x00000002
#define FILE_SHARE_DELETE 0x00000004

#define CREATE_NEW 1
#define CREATE_ALWAYS 2
#define OPEN_EXISTING 3
#define OPEN_ALWAYS 4
#define TRUNCATE_EXISTING 5

#define FILE_ATTRIBUTE_READONLY 0x00000001
#define FILE_ATTRIBUTE_HIDDEN 0x00000002
#define FILE_ATTRIBUTE_SYSTEM 0x00000004
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010
#define FILE_ATTRIBUTE_ARCHIVE 0x00000020
#define FILE_ATTRIBUTE_NORMAL 0x00000080
#define FILE_ATTRIBUTE_TEMPORARY 0x00000100
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#define INVALID_FILE_SIZE ((DWORD)-1)
#define INVALID_SET_FILE_POINTER ((DWORD)-1)

#define FILE_BEGIN SEEK_SET
#define FILE_CURRENT SEEK_CUR
#define FILE_END SEEK_END

#define INVALID_HANDLE_VALUE ((HANDLE)(ULONG_PTR)-1)

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

typedef struct _WIN32_FIND_DATAA {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD dwReserved0;
    DWORD dwReserved1;
    char cFileName[MAX_PATH];
    char cAlternateFileName[14];
} WIN32_FIND_DATAA, *PWIN32_FIND_DATAA, *LPWIN32_FIND_DATAA;

typedef WIN32_FIND_DATAA WIN32_FIND_DATA;
typedef PWIN32_FIND_DATAA PWIN32_FIND_DATA;
typedef LPWIN32_FIND_DATAA LPWIN32_FIND_DATA;

typedef struct _WIN32_FILE_ATTRIBUTE_DATA {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
} WIN32_FILE_ATTRIBUTE_DATA, *LPWIN32_FILE_ATTRIBUTE_DATA;

typedef enum _GET_FILEEX_INFO_LEVELS {
    GetFileExInfoStandard,
    GetFileExMaxInfoLevel
} GET_FILEEX_INFO_LEVELS;

typedef VOID* XMEMCOMPRESSION_CONTEXT;
typedef VOID* XMEMDECOMPRESSION_CONTEXT;

// internal search state for FindFirstFile/FindNextFile
typedef struct _LINUXSTUBS_FIND_HANDLE {
    DIR *dir;
    char dirpath[MAX_PATH];
    char pattern[MAX_PATH];
} _LINUXSTUBS_FIND_HANDLE;

#define TLS_OUT_OF_INDEXES ((DWORD)0xFFFFFFFF)

typedef pthread_mutex_t RTL_CRITICAL_SECTION;
typedef pthread_mutex_t* PRTL_CRITICAL_SECTION;

typedef RTL_CRITICAL_SECTION CRITICAL_SECTION;
typedef PRTL_CRITICAL_SECTION PCRITICAL_SECTION;
typedef PRTL_CRITICAL_SECTION LPCRITICAL_SECTION;

static inline void InitializeCriticalSection(PRTL_CRITICAL_SECTION CriticalSection)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(CriticalSection, &attr);
    pthread_mutexattr_destroy(&attr);
}

static inline void InitializeCriticalSectionAndSpinCount(PRTL_CRITICAL_SECTION CriticalSection, ULONG SpinCount)
{
    // no spin count required because we use a recursive mutex
    InitializeCriticalSection(CriticalSection);
}

static inline void DeleteCriticalSection(PRTL_CRITICAL_SECTION CriticalSection)
{
    pthread_mutex_destroy(CriticalSection);
}

static inline void EnterCriticalSection(PRTL_CRITICAL_SECTION CriticalSection)
{
    pthread_mutex_lock(CriticalSection);
}

static inline void LeaveCriticalSection(PRTL_CRITICAL_SECTION CriticalSection)
{
    pthread_mutex_unlock(CriticalSection);
}

static inline ULONG TryEnterCriticalSection(PRTL_CRITICAL_SECTION CriticalSection)
{
    return pthread_mutex_trylock(CriticalSection) == 0;
}

// https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-tlsalloc
static inline DWORD TlsAlloc(VOID)
{
    pthread_key_t key;
    if (pthread_key_create(&key, NULL) == 0)
        return key;
    return TLS_OUT_OF_INDEXES;
}

// https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-tlsfree
static inline BOOL TlsFree(DWORD dwTlsIndex)
{
    return pthread_key_delete(dwTlsIndex) == 0;
}

// https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-tlsgetvalue
static inline LPVOID TlsGetValue(DWORD dwTlsIndex)
{
    return pthread_getspecific(dwTlsIndex);
}

// https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-tlssetvalue
static inline BOOL TlsSetValue(DWORD dwTlsIndex, LPVOID lpTlsValue)
{
    return pthread_setspecific(dwTlsIndex, lpTlsValue) == 0;
}

// https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-globalmemorystatus
static inline VOID GlobalMemoryStatus(LPMEMORYSTATUS lpBuffer)
{
    // TODO: Parse /proc/meminfo and set lpBuffer based on that. Probably will also need another
    // different codepath for macOS too.
}

static inline DWORD GetLastError(VOID)
{
    return errno;
}

static inline VOID Sleep(DWORD dwMilliseconds)
{
    struct timespec ts;
    ts.tv_nsec = (dwMilliseconds * 1000000) % 1000000000;
    ts.tv_sec = dwMilliseconds / 1000;

    int ret;
    do {
        ret = nanosleep(&ts, &ts);
    } while (ret == -1 && errno == EINTR);
}

static inline LONG64 InterlockedCompareExchangeRelease64(
    LONG64 volatile *Destination,
    LONG64 Exchange,
    LONG64 Comperand)
{
    LONG64 expected = Comperand;
    __atomic_compare_exchange_n(Destination, &expected, Exchange, false, __ATOMIC_RELEASE, __ATOMIC_RELAXED);
    return expected;
}

// internal helper: convert time_t to FILETIME (100ns intervals since 1601-01-01)
static inline FILETIME _TimeToFileTime(time_t t)
{
    const ULONGLONG EPOCH_DIFF = 11644473600ULL;
    ULONGLONG val = ((ULONGLONG)t + EPOCH_DIFF) * 10000000ULL;
    FILETIME ft;
    ft.dwLowDateTime = (DWORD)(val & 0xFFFFFFFF);
    ft.dwHighDateTime = (DWORD)(val >> 32);
    return ft;
}

// internal helper: fill WIN32_FIND_DATAA from stat + name
static inline void _FillFindData(const char *name, const struct stat *st, WIN32_FIND_DATAA *out)
{
    memset(out, 0, sizeof(*out));
    out->dwFileAttributes = S_ISDIR(st->st_mode) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
    if (!(st->st_mode & S_IWUSR)) out->dwFileAttributes |= FILE_ATTRIBUTE_READONLY;
    if (name[0] == '.') out->dwFileAttributes |= FILE_ATTRIBUTE_HIDDEN;
    out->ftCreationTime = _TimeToFileTime(st->st_mtime);
    out->ftLastAccessTime = _TimeToFileTime(st->st_atime);
    out->ftLastWriteTime = _TimeToFileTime(st->st_mtime);
    out->nFileSizeHigh = (DWORD)((st->st_size >> 32) & 0xFFFFFFFF);
    out->nFileSizeLow = (DWORD)(st->st_size & 0xFFFFFFFF);
    strncpy(out->cFileName, name, MAX_PATH - 1);
}

static inline HANDLE CreateFileA(const char *lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
    void *lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
    int flags = 0;
    if ((dwDesiredAccess & GENERIC_READ) && (dwDesiredAccess & GENERIC_WRITE)) flags = O_RDWR;
    else if (dwDesiredAccess & GENERIC_WRITE) flags = O_WRONLY;
    else flags = O_RDONLY;

    switch (dwCreationDisposition)
    {
        case CREATE_NEW: flags |= O_CREAT | O_EXCL; break;
        case CREATE_ALWAYS: flags |= O_CREAT | O_TRUNC; break;
        case OPEN_EXISTING: break;
        case OPEN_ALWAYS: flags |= O_CREAT; break;
        case TRUNCATE_EXISTING: flags |= O_TRUNC; break;
    }

    int fd = open(lpFileName, flags, 0644);
    return fd == -1 ? INVALID_HANDLE_VALUE : (HANDLE)(intptr_t)fd;
}

static inline HANDLE CreateFile(const char *lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
    void *lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
    return CreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

static inline BOOL CloseHandle(HANDLE hObject)
{
    if (hObject == INVALID_HANDLE_VALUE) return FALSE;
    return close((int)(intptr_t)hObject) == 0;
}

static inline DWORD GetFileSize(HANDLE hFile, DWORD *lpFileSizeHigh)
{
    struct stat st{};
    if (fstat((int)(intptr_t)hFile, &st) != 0) { if (lpFileSizeHigh) *lpFileSizeHigh = 0; return INVALID_FILE_SIZE; }
    if (lpFileSizeHigh) *lpFileSizeHigh = (DWORD)((st.st_size >> 32) & 0xFFFFFFFF);
    return (DWORD)(st.st_size & 0xFFFFFFFF);
}

static inline BOOL GetFileSizeEx(HANDLE hFile, LARGE_INTEGER *lpFileSize)
{
    struct stat st{};
    if (fstat((int)(intptr_t)hFile, &st) != 0) return FALSE;
    if (lpFileSize) { lpFileSize->QuadPart = st.st_size; lpFileSize->LowPart = (DWORD)(st.st_size & 0xFFFFFFFF); lpFileSize->HighPart = (LONG)(st.st_size >> 32); }
    return TRUE;
}

static inline BOOL ReadFile(HANDLE hFile, void *lpBuffer, DWORD nNumberOfBytesToRead, DWORD *lpNumberOfBytesRead, void *lpOverlapped)
{
    ssize_t n = read((int)(intptr_t)hFile, lpBuffer, nNumberOfBytesToRead);
    if (lpNumberOfBytesRead) *lpNumberOfBytesRead = n >= 0 ? (DWORD)n : 0;
    return n >= 0;
}

static inline BOOL WriteFile(HANDLE hFile, const void *lpBuffer, DWORD nNumberOfBytesToWrite, DWORD *lpNumberOfBytesWritten, void *lpOverlapped)
{
    ssize_t n = write((int)(intptr_t)hFile, lpBuffer, nNumberOfBytesToWrite);
    if (lpNumberOfBytesWritten) *lpNumberOfBytesWritten = n >= 0 ? (DWORD)n : 0;
    return n >= 0;
}

static inline DWORD SetFilePointer(HANDLE hFile, LONG lDistanceToMove, LONG *lpDistanceToMoveHigh, DWORD dwMoveMethod)
{
    off_t offset = lDistanceToMove;
    if (lpDistanceToMoveHigh) offset |= ((off_t)*lpDistanceToMoveHigh << 32);
    off_t result = lseek((int)(intptr_t)hFile, offset, dwMoveMethod);
    if (result == (off_t)-1) { if (lpDistanceToMoveHigh) *lpDistanceToMoveHigh = -1; return INVALID_SET_FILE_POINTER; }
    if (lpDistanceToMoveHigh) *lpDistanceToMoveHigh = (LONG)(result >> 32);
    return (DWORD)(result & 0xFFFFFFFF);
}

static inline DWORD GetFileAttributesA(const char *lpFileName)
{
    struct stat st{};
    if (stat(lpFileName, &st) != 0) return INVALID_FILE_ATTRIBUTES;
    DWORD attrs = S_ISDIR(st.st_mode) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
    if (!(st.st_mode & S_IWUSR)) attrs |= FILE_ATTRIBUTE_READONLY;
    const char *base = strrchr(lpFileName, '/'); base = base ? base + 1 : lpFileName;
    if (base[0] == '.') attrs |= FILE_ATTRIBUTE_HIDDEN;
    return attrs;
}

static inline DWORD GetFileAttributes(const char *lpFileName)
{
    return GetFileAttributesA(lpFileName);
}

static inline BOOL GetFileAttributesExA(const char *lpFileName, GET_FILEEX_INFO_LEVELS fInfoLevelId, void *lpFileInformation)
{
    if (fInfoLevelId != GetFileExInfoStandard || !lpFileInformation) return FALSE;
    struct stat st{};
    if (stat(lpFileName, &st) != 0) return FALSE;
    WIN32_FILE_ATTRIBUTE_DATA *out = (WIN32_FILE_ATTRIBUTE_DATA *)lpFileInformation;
    out->dwFileAttributes = GetFileAttributesA(lpFileName);
    out->ftCreationTime = _TimeToFileTime(st.st_mtime);
    out->ftLastAccessTime = _TimeToFileTime(st.st_atime);
    out->ftLastWriteTime = _TimeToFileTime(st.st_mtime);
    out->nFileSizeHigh = (DWORD)((st.st_size >> 32) & 0xFFFFFFFF);
    out->nFileSizeLow = (DWORD)(st.st_size & 0xFFFFFFFF);
    return TRUE;
}

static inline BOOL GetFileAttributesEx(const char *lpFileName, GET_FILEEX_INFO_LEVELS fInfoLevelId, void *lpFileInformation)
{
    return GetFileAttributesExA(lpFileName, fInfoLevelId, lpFileInformation);
}

static inline BOOL CreateDirectoryA(const char *lpPathName, void *lpSecurityAttributes)
{
    return mkdir(lpPathName, 0755) == 0;
}

static inline BOOL CreateDirectory(const char *lpPathName, void *lpSecurityAttributes)
{
    return CreateDirectoryA(lpPathName, lpSecurityAttributes);
}

static inline BOOL DeleteFileA(const char *lpFileName)
{
    return unlink(lpFileName) == 0;
}

static inline BOOL DeleteFile(const char *lpFileName)
{
    return DeleteFileA(lpFileName);
}

static inline BOOL MoveFileA(const char *lpExistingFileName, const char *lpNewFileName)
{
    return rename(lpExistingFileName, lpNewFileName) == 0;
}

static inline BOOL MoveFile(const char *lpExistingFileName, const char *lpNewFileName)
{
    return MoveFileA(lpExistingFileName, lpNewFileName);
}

// https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-findfirstfilea
static inline HANDLE FindFirstFileA(const char *lpFileName, WIN32_FIND_DATAA *lpFindFileData)
{
    if (!lpFileName || !lpFindFileData) return INVALID_HANDLE_VALUE;

    char dirpath[MAX_PATH], pattern[MAX_PATH];
    const char *sep = strrchr(lpFileName, '/');
    if (sep)
    {
        size_t len = sep - lpFileName;
        if (len >= MAX_PATH) return INVALID_HANDLE_VALUE;
        strncpy(dirpath, lpFileName, len); dirpath[len] = '\0';
        strncpy(pattern, sep + 1, MAX_PATH - 1);
    }
    else
    {
        strncpy(dirpath, ".", MAX_PATH - 1);
        strncpy(pattern, lpFileName, MAX_PATH - 1);
    }

    DIR *dir = opendir(dirpath);
    if (!dir) return INVALID_HANDLE_VALUE;

    _LINUXSTUBS_FIND_HANDLE *fh = (_LINUXSTUBS_FIND_HANDLE *)malloc(sizeof(_LINUXSTUBS_FIND_HANDLE));
    if (!fh) { closedir(dir); return INVALID_HANDLE_VALUE; }
    fh->dir = dir;
    strncpy(fh->dirpath, dirpath, MAX_PATH - 1);
    strncpy(fh->pattern, pattern, MAX_PATH - 1);

    struct dirent *ent;
    while ((ent = readdir(fh->dir)) != NULL)
    {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
        if (fnmatch(fh->pattern, ent->d_name, 0) == 0)
        {
            char fullpath[MAX_PATH * 2];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", fh->dirpath, ent->d_name);
            struct stat st{};
            if (stat(fullpath, &st) == 0) _FillFindData(ent->d_name, &st, lpFindFileData);
            else { memset(lpFindFileData, 0, sizeof(*lpFindFileData)); strncpy(lpFindFileData->cFileName, ent->d_name, MAX_PATH - 1); }
            return (HANDLE)fh;
        }
    }

    closedir(fh->dir); free(fh);
    return INVALID_HANDLE_VALUE;
}

static inline HANDLE FindFirstFile(const char *lpFileName, WIN32_FIND_DATAA *lpFindFileData)
{
    return FindFirstFileA(lpFileName, lpFindFileData);
}

// https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-findnextfilea
static inline BOOL FindNextFileA(HANDLE hFindFile, WIN32_FIND_DATAA *lpFindFileData)
{
    if (hFindFile == INVALID_HANDLE_VALUE || !lpFindFileData) return FALSE;
    _LINUXSTUBS_FIND_HANDLE *fh = (_LINUXSTUBS_FIND_HANDLE *)hFindFile;

    struct dirent *ent;
    while ((ent = readdir(fh->dir)) != NULL)
    {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
        if (fnmatch(fh->pattern, ent->d_name, 0) == 0)
        {
            char fullpath[MAX_PATH * 2];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", fh->dirpath, ent->d_name);
            struct stat st{};
            if (stat(fullpath, &st) == 0) _FillFindData(ent->d_name, &st, lpFindFileData);
            else { memset(lpFindFileData, 0, sizeof(*lpFindFileData)); strncpy(lpFindFileData->cFileName, ent->d_name, MAX_PATH - 1); }
            return TRUE;
        }
    }
    return FALSE;
}

static inline BOOL FindNextFile(HANDLE hFindFile, WIN32_FIND_DATAA *lpFindFileData)
{
    return FindNextFileA(hFindFile, lpFindFileData);
}

// https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-findclose
static inline BOOL FindClose(HANDLE hFindFile)
{
    if (hFindFile == INVALID_HANDLE_VALUE) return FALSE;
    _LINUXSTUBS_FIND_HANDLE *fh = (_LINUXSTUBS_FIND_HANDLE *)hFindFile;
    closedir(fh->dir); free(fh);
    return TRUE;
}

static inline DWORD GetTickCount() 
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    // milliseconds
    return (long long)ts.tv_sec * 1000 + (long long)ts.tv_nsec / 1000000;
}

static inline BOOL QueryPerformanceFrequency(LARGE_INTEGER *lpFrequency) 
{ 
	// nanoseconds
	lpFrequency->QuadPart = 1000000000;		
	return false; 
}


static inline BOOL QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    // nanoseconds
	lpPerformanceCount->QuadPart = ((long long)ts.tv_sec * 1000000000) + (long long)ts.tv_nsec;

    return true;
}

#ifndef _FINAL_BUILD
VOID OutputDebugStringW(LPCWSTR lpOutputString) 
{ 
	fwprintf(stderr, lpOutputString);
}

VOID OutputDebugString(LPCSTR lpOutputString) 
{ 
	fprintf(stderr, lpOutputString); 
}

VOID OutputDebugStringA(LPCSTR lpOutputString) 
{ 
	fprintf(stderr, lpOutputString); 
}
#endif // _CONTENT_PACKAGE

#endif // LINUXSTUBS_H