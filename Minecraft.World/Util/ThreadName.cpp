#include "../Platform/stdafx.h"
#include <cstdint>

// From Xbox documentation

typedef struct tagTHREADNAME_INFO {
    std::uint32_t dwType;      // Must be 0x1000
    const char* szName;        // Pointer to name (in user address space)
    std::uint32_t dwThreadID;  // Thread ID (-1 for caller thread)
    std::uint32_t dwFlags;     // Reserved for future use; must be zero
} THREADNAME_INFO;

void SetThreadName(std::uint32_t threadId, const char* threadName) {
    THREADNAME_INFO info;

    info.dwType = 0x1000;
    info.szName = threadName;
    info.dwThreadID = threadId;
    info.dwFlags = 0;

#if (defined _WINDOWS64 | 0)
    __try {
        RaiseException(0x406D1388, 0, sizeof(info) / sizeof(std::uint32_t),
                       reinterpret_cast<uintptr_t*>(&info));
    } __except (GetExceptionCode() == 0x406D1388 ? EXCEPTION_CONTINUE_EXECUTION
                                                 : EXCEPTION_EXECUTE_HANDLER) {
    }
#endif
}
