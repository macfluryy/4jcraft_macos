#ifndef WINAPISTUBS_H
#define WINAPISTUBS_H

#pragma once

#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include <cassert>
#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cwchar>

#define __cdecl
#define _vsnprintf_s vsnprintf;

typedef unsigned int DWORD;
typedef unsigned char BYTE;
typedef BYTE* PBYTE;
typedef int HRESULT;
typedef unsigned int UINT;
typedef void* HANDLE;
typedef int INT;
typedef unsigned int* LPDWORD;
typedef char CHAR;
typedef uintptr_t ULONG_PTR;
typedef long LONG;
typedef unsigned long long PlayerUID;
typedef DWORD WORD;
typedef DWORD* PDWORD;

typedef struct {
    DWORD LowPart;
    LONG HighPart;
    long long QuadPart;
} LARGE_INTEGER;

typedef struct {
    DWORD LowPart;
    LONG HighPart;
    long long QuadPart;
} ULARGE_INTEGER;

typedef long long LONGLONG;
typedef wchar_t *LPWSTR, *PWSTR;
typedef unsigned char boolean;  // java brainrot
#define __debugbreak()
#define CONST const
typedef unsigned long ULONG;
// typedef unsigned char byte;
typedef short SHORT;
typedef float FLOAT;

#define ERROR_SUCCESS 0L
#define ERROR_IO_PENDING 997L  // dderror
#define ERROR_CANCELLED 1223L

#define INFINITE 0xFFFFFFFF  // Infinite timeout

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
#define WAIT_TIMEOUT 258

#define FILE_FLAG_WRITE_THROUGH 0x80000000
#define FILE_FLAG_OVERLAPPED 0x40000000
#define FILE_FLAG_NO_BUFFERING 0x20000000
#define FILE_FLAG_RANDOM_ACCESS 0x10000000
#define FILE_FLAG_SEQUENTIAL_SCAN 0x08000000
#define FILE_FLAG_DELETE_ON_CLOSE 0x04000000
#define FILE_FLAG_BACKUP_SEMANTICS 0x02000000

#define FILE_ATTRIBUTE_READONLY 0x00000001
#define FILE_ATTRIBUTE_HIDDEN 0x00000002
#define FILE_ATTRIBUTE_SYSTEM 0x00000004
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010
#define FILE_ATTRIBUTE_ARCHIVE 0x00000020
#define FILE_ATTRIBUTE_NORMAL 0x00000080
#define FILE_ATTRIBUTE_TEMPORARY 0x00000100
#define INVALID_FILE_ATTRIBUTES ((DWORD) - 1)
#define INVALID_FILE_SIZE ((DWORD) - 1)
#define INVALID_SET_FILE_POINTER ((DWORD) - 1)

#define FILE_BEGIN SEEK_SET
#define FILE_CURRENT SEEK_CUR
#define FILE_END SEEK_END

#define PAGE_NOACCESS 0x01
#define PAGE_READONLY 0x02
#define PAGE_READWRITE 0x04
#define PAGE_WRITECOPY 0x08
#define PAGE_EXECUTE 0x10
#define PAGE_EXECUTE_READ 0x20
#define PAGE_EXECUTE_READWRITE 0x40
#define PAGE_EXECUTE_WRITECOPY 0x80
#define PAGE_GUARD 0x100
#define PAGE_NOCACHE 0x200
#define PAGE_WRITECOMBINE 0x400
#define PAGE_USER_READONLY 0x1000
#define PAGE_USER_READWRITE 0x2000
#define MEM_COMMIT 0x1000
#define MEM_RESERVE 0x2000
#define MEM_DECOMMIT 0x4000
#define MEM_RELEASE 0x8000
#define MEM_FREE 0x10000
#define MEM_PRIVATE 0x20000
#define MEM_RESET 0x80000
#define MEM_TOP_DOWN 0x100000
#define MEM_NOZERO 0x800000
#define MEM_LARGE_PAGES 0x20000000
#define MEM_HEAP 0x40000000
#define MEM_16MB_PAGES 0x80000000

#define THREAD_BASE_PRIORITY_LOWRT \
    15                              // value that gets a thread to LowRealtime-1
#define THREAD_BASE_PRIORITY_MAX 2  // maximum thread base priority boost
#define THREAD_BASE_PRIORITY_MIN -2    // minimum thread base priority boost
#define THREAD_BASE_PRIORITY_IDLE -15  // value that gets a thread to idle

#define THREAD_PRIORITY_LOWEST THREAD_BASE_PRIORITY_MIN
#define THREAD_PRIORITY_BELOW_NORMAL (THREAD_PRIORITY_LOWEST + 1)
#define THREAD_PRIORITY_NORMAL 0
#define THREAD_PRIORITY_HIGHEST THREAD_BASE_PRIORITY_MAX
#define THREAD_PRIORITY_ABOVE_NORMAL (THREAD_PRIORITY_HIGHEST - 1)
#define THREAD_PRIORITY_ERROR_RETURN (MAXLONG)

#define THREAD_PRIORITY_TIME_CRITICAL THREAD_BASE_PRIORITY_LOWRT
#define THREAD_PRIORITY_IDLE THREAD_BASE_PRIORITY_IDLE

#define IGNORE 0             // Ignore signal
#define INFINITE 0xFFFFFFFF  // Infinite timeout
#define STATUS_WAIT_0 ((DWORD)0x00000000L)
#define WAIT_OBJECT_0 ((STATUS_WAIT_0) + 0)
#define STATUS_PENDING ((DWORD)0x00000103L)
#define STILL_ACTIVE STATUS_PENDING

#define INVALID_HANDLE_VALUE ((HANDLE)(ULONG_PTR) - 1)

// https://learn.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-filetime
typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;

typedef struct _MEMORYSTATUS {
    DWORD dwLength;
    DWORD dwMemoryLoad;
    size_t dwTotalPhys;
    size_t dwAvailPhys;
    size_t dwTotalPageFile;
    size_t dwAvailPageFile;
    size_t dwTotalVirtual;
    size_t dwAvailVirtual;
} MEMORYSTATUS, *LPMEMORYSTATUS;

typedef enum _GET_FILEEX_INFO_LEVELS {
    GetFileExInfoStandard,
    GetFileExMaxInfoLevel
} GET_FILEEX_INFO_LEVELS;

typedef void* XMEMCOMPRESSION_CONTEXT;
typedef void* XMEMDECOMPRESSION_CONTEXT;

// https://learn.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-systemtime
typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

#define TLS_OUT_OF_INDEXES ((DWORD)0xFFFFFFFF)
// https://learn.microsoft.com/en-us/cpp/c-runtime-library/truncate?view=msvc-170
#define _TRUNCATE ((size_t)-1)

#define DECLARE_HANDLE(name) typedef HANDLE name
DECLARE_HANDLE(HINSTANCE);

typedef HINSTANCE HMODULE;

#define _HRESULT_TYPEDEF_(_sc) _sc

#define FAILED(Status) ((HRESULT)(Status) < 0)
#define MAKE_HRESULT(sev, fac, code)                                       \
    ((HRESULT)(((unsigned int)(sev) << 31) | ((unsigned int)(fac) << 16) | \
               ((unsigned int)(code))))
#define MAKE_SCODE(sev, fac, code)                                       \
    ((SCODE)(((unsigned int)(sev) << 31) | ((unsigned int)(fac) << 16) | \
             ((unsigned int)(code))))
#define E_FAIL _HRESULT_TYPEDEF_(0x80004005L)
#define E_ABORT _HRESULT_TYPEDEF_(0x80004004L)
#define E_NOINTERFACE _HRESULT_TYPEDEF_(0x80004002L)

// https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-globalmemorystatus
static inline void GlobalMemoryStatus(LPMEMORYSTATUS lpBuffer) {
    // TODO: Parse /proc/meminfo and set lpBuffer based on that. Probably will
    // also need another different codepath for macOS too.
}

static inline DWORD GetLastError(void) { return errno; }

#ifdef __LP64__
static inline int64_t InterlockedCompareExchangeRelease64(
    int64_t volatile* Destination, int64_t Exchange, int64_t Comperand) {
    int64_t expected = Comperand;
    __atomic_compare_exchange_n(Destination, &expected, Exchange, false,
                                __ATOMIC_RELEASE, __ATOMIC_RELAXED);
    return expected;
}
#else
static inline int64_t InterlockedCompareExchangeRelease(
    LONG volatile* Destination, LONG Exchange, LONG Comperand) {
    LONG expected = Comperand;
    __atomic_compare_exchange_n(Destination, &expected, Exchange, false,
                                __ATOMIC_RELEASE, __ATOMIC_RELAXED);
    return expected;
}
#endif

// internal helper: convert time_t to FILETIME (100ns intervals since
// 1601-01-01)
static inline FILETIME _TimeToFileTime(time_t t) {
    const uint64_t EPOCH_DIFF = 11644473600ULL;
    uint64_t val = ((uint64_t)t + EPOCH_DIFF) * 10000000ULL;
    FILETIME ft;
    ft.dwLowDateTime = (DWORD)(val & 0xFFFFFFFF);
    ft.dwHighDateTime = (DWORD)(val >> 32);
    return ft;
}

static inline HANDLE CreateFileA(const char* lpFileName, DWORD dwDesiredAccess,
                                 DWORD dwShareMode, void* lpSecurityAttributes,
                                 DWORD dwCreationDisposition,
                                 DWORD dwFlagsAndAttributes,
                                 HANDLE hTemplateFile) {
    int flags = 0;
    if ((dwDesiredAccess & GENERIC_READ) && (dwDesiredAccess & GENERIC_WRITE))
        flags = O_RDWR;
    else if (dwDesiredAccess & GENERIC_WRITE)
        flags = O_WRONLY;
    else
        flags = O_RDONLY;

    switch (dwCreationDisposition) {
        case CREATE_NEW:
            flags |= O_CREAT | O_EXCL;
            break;
        case CREATE_ALWAYS:
            flags |= O_CREAT | O_TRUNC;
            break;
        case OPEN_EXISTING:
            break;
        case OPEN_ALWAYS:
            flags |= O_CREAT;
            break;
        case TRUNCATE_EXISTING:
            flags |= O_TRUNC;
            break;
    }

    int fd = open(lpFileName, flags, 0644);
    return fd == -1 ? INVALID_HANDLE_VALUE : (HANDLE)(intptr_t)fd;
}

static inline HANDLE CreateFileW(const wchar_t* lpFileName,
                                 DWORD dwDesiredAccess, DWORD dwShareMode,
                                 void* lpSecurityAttributes,
                                 DWORD dwCreationDisposition,
                                 DWORD dwFlagsAndAttributes,
                                 HANDLE hTemplateFile) {
    char narrowBuf[1024];
    wcstombs(narrowBuf, lpFileName, sizeof(narrowBuf));
    narrowBuf[sizeof(narrowBuf) - 1] = '\0';
    return CreateFileA(narrowBuf, dwDesiredAccess, dwShareMode,
                       lpSecurityAttributes, dwCreationDisposition,
                       dwFlagsAndAttributes, hTemplateFile);
}

static inline HANDLE CreateFile(const char* lpFileName, DWORD dwDesiredAccess,
                                DWORD dwShareMode, void* lpSecurityAttributes,
                                DWORD dwCreationDisposition,
                                DWORD dwFlagsAndAttributes,
                                HANDLE hTemplateFile) {
    return CreateFileA(lpFileName, dwDesiredAccess, dwShareMode,
                       lpSecurityAttributes, dwCreationDisposition,
                       dwFlagsAndAttributes, hTemplateFile);
}

static inline HANDLE CreateFile(const wchar_t* lpFileName,
                                DWORD dwDesiredAccess, DWORD dwShareMode,
                                void* lpSecurityAttributes,
                                DWORD dwCreationDisposition,
                                DWORD dwFlagsAndAttributes,
                                HANDLE hTemplateFile) {
    return CreateFileW(lpFileName, dwDesiredAccess, dwShareMode,
                       lpSecurityAttributes, dwCreationDisposition,
                       dwFlagsAndAttributes, hTemplateFile);
}

static inline bool CloseHandle(HANDLE hObject) {
    if (hObject == INVALID_HANDLE_VALUE) return false;
    return close((int)(intptr_t)hObject) == 0;
}

static inline DWORD GetFileSize(HANDLE hFile, DWORD* lpFileSizeHigh) {
    struct stat st{};
    if (fstat((int)(intptr_t)hFile, &st) != 0) {
        if (lpFileSizeHigh) *lpFileSizeHigh = 0;
        return INVALID_FILE_SIZE;
    }
    if (lpFileSizeHigh)
        *lpFileSizeHigh = (DWORD)((st.st_size >> 32) & 0xFFFFFFFF);
    return (DWORD)(st.st_size & 0xFFFFFFFF);
}

static inline bool GetFileSizeEx(HANDLE hFile, LARGE_INTEGER* lpFileSize) {
    struct stat st{};
    if (fstat((int)(intptr_t)hFile, &st) != 0) return false;
    if (lpFileSize) {
        lpFileSize->QuadPart = st.st_size;
        lpFileSize->LowPart = (DWORD)(st.st_size & 0xFFFFFFFF);
        lpFileSize->HighPart = (LONG)(st.st_size >> 32);
    }
    return true;
}

static inline bool ReadFile(HANDLE hFile, void* lpBuffer,
                            DWORD nNumberOfBytesToRead,
                            DWORD* lpNumberOfBytesRead, void* lpOverlapped) {
    ssize_t n = read((int)(intptr_t)hFile, lpBuffer, nNumberOfBytesToRead);
    if (lpNumberOfBytesRead) *lpNumberOfBytesRead = n >= 0 ? (DWORD)n : 0;
    return n >= 0;
}

// internal helper: convert FILETIME (100ns since 1601) to time_t (seconds since
// 1970)
static inline time_t _FileTimeToTimeT(const FILETIME& ft) {
    uint64_t val = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    const uint64_t EPOCH_DIFF =
        116444736000000000ULL;  // 100ns intervals between 1601-01-01 and
                                // 1970-01-01
    return (time_t)((val - EPOCH_DIFF) / 10000000ULL);
}

// internal helper: read the current wall clock into a timespec
static inline void _CurrentTimeSpec(struct timespec* ts) {
#ifdef CLOCK_REALTIME
    clock_gettime(CLOCK_REALTIME, ts);
#else
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    ts->tv_sec = tv.tv_sec;
    ts->tv_nsec = tv.tv_usec * 1000;
#endif
}

// internal helper: fill SYSTEMTIME from a broken-down tm + nanosecond remainder
static inline void _FillSystemTime(const struct tm* tm, long tv_nsec,
                                   LPSYSTEMTIME lpSystemTime) {
    lpSystemTime->wYear = tm->tm_year + 1900;
    lpSystemTime->wMonth = tm->tm_mon + 1;
    lpSystemTime->wDayOfWeek = tm->tm_wday;  // 0 = Sunday
    lpSystemTime->wDay = tm->tm_mday;
    lpSystemTime->wHour = tm->tm_hour;
    lpSystemTime->wMinute = tm->tm_min;
    lpSystemTime->wSecond = tm->tm_sec;
    lpSystemTime->wMilliseconds = (WORD)(tv_nsec / 1000000);  // ns to ms
}

// https://learn.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getlocaltime
static inline void GetLocalTime(LPSYSTEMTIME lpSystemTime) {
    struct timespec ts;
    _CurrentTimeSpec(&ts);
    struct tm tm;
    localtime_r(&ts.tv_sec, &tm);  // local time
    _FillSystemTime(&tm, ts.tv_nsec, lpSystemTime);
}

// https://learn.microsoft.com/en-us/windows/win32/api/timezoneapi/nf-timezoneapi-filetimetosystemtime
static inline bool FileTimeToSystemTime(const FILETIME* lpFileTime,
                                        LPSYSTEMTIME lpSystemTime) {
    uint64_t ft = ((uint64_t)lpFileTime->dwHighDateTime << 32) |
                  lpFileTime->dwLowDateTime;
    time_t t = _FileTimeToTimeT(*lpFileTime);
    long remainder_ns = (long)((ft % 10000000ULL) * 100);

    struct tm tm;
    gmtime_r(&t, &tm);  // UTC
    _FillSystemTime(&tm, remainder_ns, lpSystemTime);
    return true;
}

// https://learn.microsoft.com/en-us/windows/win32/api/debugapi/nf-debugapi-outputdebugstringa
static inline void OutputDebugStringA(const char* lpOutputString) {
    if (!lpOutputString) return;
    fputs(lpOutputString, stderr);
}

// https://learn.microsoft.com/en-us/windows/win32/api/debugapi/nf-debugapi-outputdebugstringw
static inline void OutputDebugStringW(const wchar_t* lpOutputString) {
    if (!lpOutputString) return;
    fprintf(stderr, "%ls", lpOutputString);
}

static inline void OutputDebugString(const char* lpOutputString) {
    return OutputDebugStringA(lpOutputString);
}

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int signaled;
    int manual_reset;
} Event;

static inline HANDLE CreateEvent(int manual_reset, int initial_state) {
    Event* ev = (Event*)malloc(sizeof(Event));
    pthread_mutex_init(&ev->mutex, nullptr);
    pthread_cond_init(&ev->cond, nullptr);
    ev->signaled = initial_state;
    ev->manual_reset = manual_reset;
    return (HANDLE)ev;
}

static inline HANDLE CreateEvent(void*, bool manual_reset, bool initial_state,
                                 void*) {
    return CreateEvent(manual_reset, initial_state);
}

static inline bool SetEvent(HANDLE hEvent) {
    Event* ev = (Event*)hEvent;
    if (!ev) return false;
    pthread_mutex_lock(&ev->mutex);
    ev->signaled = 1;
    if (ev->manual_reset)
        pthread_cond_broadcast(&ev->cond);
    else
        pthread_cond_signal(&ev->cond);
    pthread_mutex_unlock(&ev->mutex);
    return true;
}

static inline bool ResetEvent(HANDLE hEvent) {
    Event* ev = (Event*)hEvent;
    if (!ev) return false;
    pthread_mutex_lock(&ev->mutex);
    ev->signaled = 0;
    pthread_mutex_unlock(&ev->mutex);
    return true;
}

#define WAIT_FAILED ((DWORD)0xFFFFFFFF)
#define INFINITE 0xFFFFFFFF
#define HANDLE_TYPE_THREAD 0x54485200

static inline DWORD _WaitForEvent(Event* ev, DWORD dwMilliseconds) {
    pthread_mutex_lock(&ev->mutex);
    if (dwMilliseconds == INFINITE) {
        while (!ev->signaled) pthread_cond_wait(&ev->cond, &ev->mutex);
    } else if (dwMilliseconds > 0) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += dwMilliseconds / 1000;
        ts.tv_nsec += (dwMilliseconds % 1000) * 1000000;
        if (ts.tv_nsec >= 1000000000) {
            ts.tv_sec++;
            ts.tv_nsec -= 1000000000;
        }
        while (!ev->signaled) {
            if (pthread_cond_timedwait(&ev->cond, &ev->mutex, &ts) != 0) {
                pthread_mutex_unlock(&ev->mutex);
                return WAIT_TIMEOUT;
            }
        }
    } else {
        if (!ev->signaled) {
            pthread_mutex_unlock(&ev->mutex);
            return WAIT_TIMEOUT;
        }
    }
    if (!ev->manual_reset) ev->signaled = 0;
    pthread_mutex_unlock(&ev->mutex);
    return WAIT_OBJECT_0;
}

struct LinuxThread;
static inline DWORD _WaitForThread(struct LinuxThread* lt,
                                   DWORD dwMilliseconds);

static inline DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds) {
    if (!hHandle) return WAIT_FAILED;
    // Check if this is a thread handle (LinuxThread has magic number as first
    // field)
    if (*(int*)hHandle == HANDLE_TYPE_THREAD) {
        return _WaitForThread((struct LinuxThread*)hHandle, dwMilliseconds);
    }
    return _WaitForEvent((Event*)hHandle, dwMilliseconds);
}

static inline DWORD WaitForMultipleObjects(DWORD nCount,
                                           const HANDLE* lpHandles,
                                           bool bWaitAll,
                                           DWORD dwMilliseconds) {
    if (bWaitAll) {
        for (DWORD i = 0; i < nCount; i++)
            WaitForSingleObject(lpHandles[i], dwMilliseconds);
        return WAIT_OBJECT_0;
    }
    for (int pass = 0; pass < 1000; pass++) {
        for (DWORD i = 0; i < nCount; i++) {
            if (WaitForSingleObject(lpHandles[i], 0) == WAIT_OBJECT_0)
                return WAIT_OBJECT_0 + i;
        }
        usleep(1000);
    }
    return WAIT_TIMEOUT;
}

static inline void CloseHandle_Event(HANDLE hEvent) {
    Event* ev = (Event*)hEvent;
    if (!ev) return;
    pthread_mutex_destroy(&ev->mutex);
    pthread_cond_destroy(&ev->cond);
    free(ev);
}

#define CREATE_SUSPENDED 0x00000004

typedef DWORD (*LPTHREAD_START_ROUTINE)(void*);

struct LinuxThread {
    int handleType;
    pthread_t thread;
    LPTHREAD_START_ROUTINE func;
    void* param;
    DWORD threadId;
    DWORD exitCode;
    int suspended;
    pthread_mutex_t suspendMutex;
    pthread_cond_t suspendCond;
    pthread_mutex_t completionMutex;
    pthread_cond_t completionCond;
    int completed;
};

static inline void* _linux_thread_entry(void* arg) {
    LinuxThread* lt = (LinuxThread*)arg;
    pthread_mutex_lock(&lt->suspendMutex);
    while (lt->suspended)
        pthread_cond_wait(&lt->suspendCond, &lt->suspendMutex);
    pthread_mutex_unlock(&lt->suspendMutex);
    lt->exitCode = lt->func(lt->param);
    // Signal completion
    pthread_mutex_lock(&lt->completionMutex);
    lt->completed = 1;
    pthread_cond_broadcast(&lt->completionCond);
    pthread_mutex_unlock(&lt->completionMutex);
    return nullptr;
}

static inline DWORD _WaitForThread(struct LinuxThread* lt,
                                   DWORD dwMilliseconds) {
    pthread_mutex_lock(&lt->completionMutex);
    if (lt->completed) {
        pthread_mutex_unlock(&lt->completionMutex);
        return WAIT_OBJECT_0;
    }
    if (dwMilliseconds == 0) {
        pthread_mutex_unlock(&lt->completionMutex);
        return WAIT_TIMEOUT;
    }
    if (dwMilliseconds == INFINITE) {
        while (!lt->completed)
            pthread_cond_wait(&lt->completionCond, &lt->completionMutex);
    } else {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += dwMilliseconds / 1000;
        ts.tv_nsec += (dwMilliseconds % 1000) * 1000000;
        if (ts.tv_nsec >= 1000000000) {
            ts.tv_sec++;
            ts.tv_nsec -= 1000000000;
        }
        while (!lt->completed) {
            if (pthread_cond_timedwait(&lt->completionCond,
                                       &lt->completionMutex, &ts) != 0) {
                pthread_mutex_unlock(&lt->completionMutex);
                return WAIT_TIMEOUT;
            }
        }
    }
    pthread_mutex_unlock(&lt->completionMutex);
    return WAIT_OBJECT_0;
}

static DWORD g_nextThreadId = 1000;

static inline HANDLE CreateThread(void*, size_t stackSize,
                                  LPTHREAD_START_ROUTINE lpStartAddress,
                                  void* lpParameter, DWORD dwCreationFlags,
                                  DWORD* lpThreadId) {
    LinuxThread* lt = (LinuxThread*)calloc(1, sizeof(LinuxThread));
    lt->handleType = HANDLE_TYPE_THREAD;
    lt->func = lpStartAddress;
    lt->param = lpParameter;
    lt->exitCode = STILL_ACTIVE;
    lt->suspended = (dwCreationFlags & CREATE_SUSPENDED) ? 1 : 0;
    lt->completed = 0;
    lt->threadId = __sync_fetch_and_add(&g_nextThreadId, 1);
    pthread_mutex_init(&lt->suspendMutex, nullptr);
    pthread_cond_init(&lt->suspendCond, nullptr);
    pthread_mutex_init(&lt->completionMutex, nullptr);
    pthread_cond_init(&lt->completionCond, nullptr);
    if (lpThreadId) *lpThreadId = lt->threadId;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    if (stackSize > 0) pthread_attr_setstacksize(&attr, stackSize);
    pthread_create(&lt->thread, &attr, _linux_thread_entry, lt);
    pthread_attr_destroy(&attr);
    return (HANDLE)lt;
}

static inline DWORD ResumeThread(HANDLE hThread) {
    LinuxThread* lt = (LinuxThread*)hThread;
    if (!lt) return (DWORD)-1;
    pthread_mutex_lock(&lt->suspendMutex);
    lt->suspended = 0;
    pthread_cond_signal(&lt->suspendCond);
    pthread_mutex_unlock(&lt->suspendMutex);
    return 0;
}

static inline bool SetThreadPriority(HANDLE hThread, int nPriority) {
    (void)hThread;
    (void)nPriority;
    return true;
}

static inline bool GetExitCodeThread(HANDLE hThread, DWORD* lpExitCode) {
    LinuxThread* lt = (LinuxThread*)hThread;
    if (!lt || !lpExitCode) return false;
    *lpExitCode = lt->exitCode;
    return true;
}

static inline DWORD GetCurrentThreadId() {
    return (DWORD)(unsigned long)pthread_self();
}

static inline HANDLE GetCurrentThread() {
    return (HANDLE)(unsigned long)pthread_self();
}

template <size_t N>
static inline int swprintf_s(wchar_t (&buf)[N], const wchar_t* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int ret = vswprintf(buf, N, fmt, args);
    va_end(args);
    return ret;
}

static inline int swprintf_s(wchar_t* buf, size_t sz, const wchar_t* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int ret = vswprintf(buf, sz, fmt, args);
    va_end(args);
    return ret;
}

static inline HMODULE GetModuleHandle(const char* lpModuleName) { return 0; }

static inline void* VirtualAlloc(void* lpAddress, size_t dwSize,
                                 DWORD flAllocationType, DWORD flProtect) {
    // MEM_COMMIT | MEM_RESERVE → mmap anonymous
    int prot = 0;
    if (flProtect == 0x04 /*PAGE_READWRITE*/)
        prot = PROT_READ | PROT_WRITE;
    else if (flProtect == 0x40 /*PAGE_EXECUTE_READWRITE*/)
        prot = PROT_READ | PROT_WRITE | PROT_EXEC;
    else if (flProtect == 0x02 /*PAGE_READONLY*/)
        prot = PROT_READ;
    else
        prot = PROT_READ | PROT_WRITE;  // default

    int flags = MAP_PRIVATE | MAP_ANONYMOUS;
    if (lpAddress != nullptr) flags |= MAP_FIXED;

    void* p = mmap(lpAddress, dwSize, prot, flags, -1, 0);
    if (p == MAP_FAILED) return nullptr;
    return p;
}

static inline bool VirtualFree(void* lpAddress, size_t dwSize,
                               DWORD dwFreeType) {
    if (lpAddress == nullptr) return false;
    // MEM_RELEASE (0x8000) frees the whole region
    if (dwFreeType == 0x8000 /*MEM_RELEASE*/) {
        // dwSize should be 0 for MEM_RELEASE per Win32 API, but we don't track
        // allocation sizes Use dwSize if provided, otherwise this is a
        // best-effort
        if (dwSize == 0) dwSize = 4096;  // minimum page
        munmap(lpAddress, dwSize);
    } else {
        // MEM_DECOMMIT (0x4000) - just decommit (make inaccessible)
        madvise(lpAddress, dwSize, MADV_DONTNEED);
    }
    return true;
}

#define _wcsicmp wcscasecmp

#endif  // WINAPISTUBS_H
