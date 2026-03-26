#include "../Platform/stdafx.h"

#include "C4JThread.h"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cerrno>
#include <chrono>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <thread>
#include <vector>

#if defined(_WIN32)
#include <Windows.h>
#endif

#if defined(__linux__)
#include <pthread.h>
#include <sched.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "../../Minecraft.Client/Platform/Common/ShutdownManager.h"

#if !defined(INFINITE)
#define INFINITE 0xFFFFFFFFu
#endif

#if !defined(WAIT_OBJECT_0)
#define WAIT_OBJECT_0 0u
#endif

#if !defined(WAIT_TIMEOUT)
#define WAIT_TIMEOUT 258u
#endif

#if !defined(STILL_ACTIVE)
#define STILL_ACTIVE 259
#endif

#if !defined(THREAD_PRIORITY_IDLE)
#define THREAD_PRIORITY_IDLE (-15)
#endif

#if !defined(THREAD_PRIORITY_LOWEST)
#define THREAD_PRIORITY_LOWEST (-2)
#endif

#if !defined(THREAD_PRIORITY_BELOW_NORMAL)
#define THREAD_PRIORITY_BELOW_NORMAL (-1)
#endif

#if !defined(THREAD_PRIORITY_NORMAL)
#define THREAD_PRIORITY_NORMAL 0
#endif

#if !defined(THREAD_PRIORITY_ABOVE_NORMAL)
#define THREAD_PRIORITY_ABOVE_NORMAL 1
#endif

#if !defined(THREAD_PRIORITY_HIGHEST)
#define THREAD_PRIORITY_HIGHEST 2
#endif

#if !defined(THREAD_PRIORITY_TIME_CRITICAL)
#define THREAD_PRIORITY_TIME_CRITICAL 15
#endif

thread_local C4JThread* C4JThread::ms_currentThread = nullptr;

namespace {

constexpr int kDefaultStackSize = 65536 * 2;
constexpr int kMinimumStackSize = 16384;
constexpr int kUnsetPriority = std::numeric_limits<int>::min();
constexpr int kEventQueueShutdownPollMs = 100;

const std::thread::id g_processMainThreadId = std::this_thread::get_id();

template <typename Predicate>
bool WaitForCondition(std::condition_variable& condition,
                      std::unique_lock<std::mutex>& lock,
                      int timeoutMs,
                      Predicate predicate) {
    if (timeoutMs < 0 ||
        static_cast<std::uint32_t>(timeoutMs) ==
            static_cast<std::uint32_t>(INFINITE)) {
        condition.wait(lock, predicate);
        return true;
    }

    return condition.wait_for(lock, std::chrono::milliseconds(timeoutMs),
                              predicate);
}

std::uint32_t FirstSetBitIndex(std::uint32_t bitMask) {
    return static_cast<std::uint32_t>(std::countr_zero(bitMask));
}

std::uint32_t BuildMaskForSize(int size) {
    assert(size > 0);
    assert(size <= 32);

    if (size == 32) {
        return 0xFFFFFFFFU;
    }

    return (1U << static_cast<std::uint32_t>(size)) - 1U;
}

void FormatThreadName(std::string& outThreadName, const char* threadName) {
    const char* safeName =
        (threadName != nullptr && threadName[0] != '\0') ? threadName : "Unnamed";

    char buffer[64];
    std::snprintf(buffer, sizeof(buffer), "(4J) %s", safeName);
    buffer[sizeof(buffer) - 1] = '\0';
    outThreadName = buffer;
}

const char* GetSafeThreadName(const char* threadName) {
    return (threadName != nullptr && threadName[0] != '\0') ? threadName
                                                            : "(4J) Unnamed";
}

bool IsProcessorIndexPlausible(int proc) {
    if (proc < 0) {
        return true;
    }

    const unsigned int hardwareThreads = std::thread::hardware_concurrency();
    if (hardwareThreads == 0U) {
        return true;
    }

    return static_cast<unsigned int>(proc) < hardwareThreads;
}

#if defined(__linux__)
std::int64_t GetLinuxThreadId() {
    return static_cast<std::int64_t>(::syscall(SYS_gettid));
}

int MapPriorityToNice(int priority) {
    switch (priority) {
    case THREAD_PRIORITY_TIME_CRITICAL:
        return -15;
    case THREAD_PRIORITY_HIGHEST:
        return -10;
    case THREAD_PRIORITY_ABOVE_NORMAL:
        return -5;
    case THREAD_PRIORITY_NORMAL:
        return 0;
    case THREAD_PRIORITY_BELOW_NORMAL:
        return 5;
    case THREAD_PRIORITY_LOWEST:
        return 10;
    case THREAD_PRIORITY_IDLE:
        return 19;
    default:
        return 0;
    }
}
#endif

#if defined(_WIN32)
thread_local std::vector<DWORD_PTR> g_affinityMaskStack;
#elif defined(__linux__)
thread_local std::vector<cpu_set_t> g_affinityMaskStack;
#endif

}  // namespace

C4JThread::C4JThread(C4JThreadStartFunc* startFunc, void* param,
                     const char* threadName, int stackSize)
    : m_threadParam(param),
      m_startFunc(startFunc),
      m_stackSize(stackSize == 0 ? kDefaultStackSize : stackSize),
      m_threadName(),
      m_isRunning(false),
      m_hasStarted(false),
      m_exitCode(STILL_ACTIVE),
      m_threadID(),
      m_threadHandle(),
      m_completionFlag(std::make_unique<Event>(Event::e_modeManualClear)),
      m_requestedProcessor(-1),
      m_requestedPriority(kUnsetPriority),
      m_nativeTid(0) {
    if (m_stackSize < kMinimumStackSize) {
        m_stackSize = kMinimumStackSize;
    }

    FormatThreadName(m_threadName, threadName);
}

C4JThread::C4JThread(const char* mainThreadName)
    : m_threadParam(nullptr),
      m_startFunc(nullptr),
      m_stackSize(0),
      m_threadName(),
      m_isRunning(true),
      m_hasStarted(true),
      m_exitCode(STILL_ACTIVE),
      m_threadID(std::this_thread::get_id()),
      m_threadHandle(),
      m_completionFlag(std::make_unique<Event>(Event::e_modeManualClear)),
      m_requestedProcessor(-1),
      m_requestedPriority(kUnsetPriority),
#if defined(__linux__)
      m_nativeTid(GetLinuxThreadId())
#else
      m_nativeTid(0)
#endif
{
    FormatThreadName(m_threadName, mainThreadName);
    ms_currentThread = this;
    SetCurrentThreadName(m_threadName.c_str());
}

C4JThread::~C4JThread() {
    if (m_threadHandle.joinable()) {
        if (m_threadHandle.get_id() == std::this_thread::get_id()) {
            m_threadHandle.detach();
        } else {
            m_threadHandle.join();
        }
    }

    if (ms_currentThread == this) {
        ms_currentThread = nullptr;
    }
}

C4JThread& C4JThread::getMainThreadInstance() noexcept {
    static C4JThread mainThread("Main thread");
    return mainThread;
}

void C4JThread::entryPoint(C4JThread* pThread) {
    ms_currentThread = pThread;
    pThread->m_threadID = std::this_thread::get_id();

#if defined(__linux__)
    pThread->m_nativeTid.store(GetLinuxThreadId(), std::memory_order_release);
#endif

    SetCurrentThreadName(pThread->m_threadName.c_str());

    const int requestedProcessor =
        pThread->m_requestedProcessor.load(std::memory_order_acquire);
    if (requestedProcessor >= 0) {
        pThread->SetProcessor(requestedProcessor);
    }

    const int requestedPriority =
        pThread->m_requestedPriority.load(std::memory_order_acquire);
    if (requestedPriority != kUnsetPriority) {
        pThread->SetPriority(requestedPriority);
    }

    int exitCode = 0;
    try {
        exitCode =
            (pThread->m_startFunc != nullptr)
                ? (*pThread->m_startFunc)(pThread->m_threadParam)
                : 0;
    } catch (...) {
        exitCode = -1;
    }

    pThread->m_exitCode.store(exitCode, std::memory_order_release);
    pThread->m_isRunning.store(false, std::memory_order_release);
    pThread->m_completionFlag->Set();
}

void C4JThread::Run() {
    bool expected = false;
    if (!m_hasStarted.compare_exchange_strong(expected, true,
                                              std::memory_order_acq_rel)) {
        return;
    }

    m_isRunning.store(true, std::memory_order_release);
    m_exitCode.store(STILL_ACTIVE, std::memory_order_release);
    m_completionFlag->Clear();
    m_nativeTid.store(0, std::memory_order_release);

    m_threadHandle = std::thread(&C4JThread::entryPoint, this);
    m_threadID = m_threadHandle.get_id();

    const int requestedProcessor =
        m_requestedProcessor.load(std::memory_order_acquire);
    if (requestedProcessor >= 0) {
        SetProcessor(requestedProcessor);
    }

    const int requestedPriority =
        m_requestedPriority.load(std::memory_order_acquire);
    if (requestedPriority != kUnsetPriority) {
        SetPriority(requestedPriority);
    }
}

void C4JThread::SetProcessor(int proc) {
    m_requestedProcessor.store(proc, std::memory_order_release);

    if (!IsProcessorIndexPlausible(proc)) {
        return;
    }

#if defined(_WIN32)
    HANDLE threadHandle = nullptr;

    if (m_threadHandle.joinable()) {
        threadHandle = m_threadHandle.native_handle();
    } else if (ms_currentThread == this) {
        threadHandle = ::GetCurrentThread();
    } else {
        return;
    }

    DWORD_PTR affinityMask = 0;
    if (proc < 0) {
        DWORD_PTR processAffinityMask = 0;
        DWORD_PTR systemAffinityMask = 0;
        if (!::GetProcessAffinityMask(::GetCurrentProcess(),
                                      &processAffinityMask,
                                      &systemAffinityMask) ||
            processAffinityMask == 0) {
            return;
        }
        affinityMask = processAffinityMask;
    } else {
        const unsigned int bitCount =
            static_cast<unsigned int>(sizeof(DWORD_PTR) * CHAR_BIT);
        if (static_cast<unsigned int>(proc) >= bitCount) {
            return;
        }

        affinityMask =
            (static_cast<DWORD_PTR>(1) << static_cast<unsigned int>(proc));
    }

    (void)::SetThreadAffinityMask(threadHandle, affinityMask);

#elif defined(__linux__)
    pthread_t threadHandle;

    if (m_threadHandle.joinable()) {
        threadHandle = m_threadHandle.native_handle();
    } else if (ms_currentThread == this) {
        threadHandle = ::pthread_self();
    } else {
        return;
    }

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);

    if (proc < 0) {
        if (::sched_getaffinity(0, sizeof(cpuset), &cpuset) != 0) {
            return;
        }
    } else {
        if (proc >= CPU_SETSIZE) {
            return;
        }
        CPU_SET(proc, &cpuset);
    }

    (void)::pthread_setaffinity_np(threadHandle, sizeof(cpuset), &cpuset);
#else
    (void)proc;
#endif
}

void C4JThread::SetPriority(int priority) {
    m_requestedPriority.store(priority, std::memory_order_release);

#if defined(_WIN32)
    HANDLE threadHandle = nullptr;

    if (m_threadHandle.joinable()) {
        threadHandle = m_threadHandle.native_handle();
    } else if (ms_currentThread == this) {
        threadHandle = ::GetCurrentThread();
    } else {
        return;
    }

    (void)::SetThreadPriority(threadHandle, priority);

#elif defined(__linux__)
    std::int64_t nativeTid = 0;

    if (ms_currentThread == this) {
        nativeTid = GetLinuxThreadId();
        m_nativeTid.store(nativeTid, std::memory_order_release);
    } else {
        nativeTid = m_nativeTid.load(std::memory_order_acquire);
    }

    if (nativeTid <= 0) {
        return;
    }

    const int niceValue = MapPriorityToNice(priority);

    errno = 0;
    if (::setpriority(PRIO_PROCESS, static_cast<id_t>(nativeTid), niceValue) !=
        0) {
        if ((errno == EACCES || errno == EPERM) && niceValue < 0) {
            (void)::setpriority(PRIO_PROCESS, static_cast<id_t>(nativeTid), 0);
        }
    }
#else
    (void)priority;
#endif
}

std::uint32_t C4JThread::WaitForCompletion(int timeoutMs) {
    const std::uint32_t waitResult = m_completionFlag->WaitForSignal(timeoutMs);
    if (waitResult == WAIT_OBJECT_0 && m_threadHandle.joinable()) {
        if (m_threadHandle.get_id() == std::this_thread::get_id()) {
            m_threadHandle.detach();
        } else {
            m_threadHandle.join();
        }
    }
    return waitResult;
}

int C4JThread::GetExitCode() const noexcept {
    return m_isRunning.load(std::memory_order_acquire)
               ? STILL_ACTIVE
               : m_exitCode.load(std::memory_order_acquire);
}

void C4JThread::Sleep(int millisecs) {
    if (millisecs <= 0) {
        std::this_thread::yield();
        return;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(millisecs));
}

C4JThread* C4JThread::getCurrentThread() noexcept {
    if (ms_currentThread != nullptr) {
        return ms_currentThread;
    }

    if (std::this_thread::get_id() == g_processMainThreadId) {
        return &getMainThreadInstance();
    }

    return nullptr;
}

bool C4JThread::isMainThread() noexcept {
    return std::this_thread::get_id() == g_processMainThreadId;
}

void C4JThread::SetThreadName(std::uint32_t threadId, const char* threadName) {
    const char* safeThreadName = GetSafeThreadName(threadName);

#if defined(_WIN32)
    if (threadId == static_cast<std::uint32_t>(-1) ||
        threadId == ::GetCurrentThreadId()) {
        using SetThreadDescriptionFn = HRESULT(WINAPI*)(HANDLE, PCWSTR);

        const HMODULE kernelModule = ::GetModuleHandleW(L"Kernel32.dll");
        if (kernelModule != nullptr) {
            const auto setThreadDescription =
                reinterpret_cast<SetThreadDescriptionFn>(
                    ::GetProcAddress(kernelModule, "SetThreadDescription"));

            if (setThreadDescription != nullptr) {
                wchar_t wideName[64];
                const std::size_t converted =
                    std::mbstowcs(wideName, safeThreadName,
                                  (sizeof(wideName) / sizeof(wideName[0])) - 1);

                if (converted != static_cast<std::size_t>(-1)) {
                    wideName[converted] = L'\0';
                    (void)setThreadDescription(::GetCurrentThread(), wideName);
                    return;
                }
            }
        }
    }

#pragma pack(push, 8)
    struct THREADNAME_INFO {
        std::uint32_t dwType;
        const char* szName;
        std::uint32_t dwThreadID;
        std::uint32_t dwFlags;
    };
#pragma pack(pop)

    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = safeThreadName;
    info.dwThreadID = threadId;
    info.dwFlags = 0;

    __try {
        ::RaiseException(0x406D1388, 0, sizeof(info) / sizeof(ULONG_PTR),
                         reinterpret_cast<ULONG_PTR*>(&info));
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }

#elif defined(__linux__)
    (void)threadId;

    char truncatedName[16];
    std::snprintf(truncatedName, sizeof(truncatedName), "%s", safeThreadName);
    truncatedName[sizeof(truncatedName) - 1] = '\0';

    (void)::pthread_setname_np(::pthread_self(), truncatedName);

#else
    (void)threadId;
    (void)safeThreadName;
#endif
}

void C4JThread::SetCurrentThreadName(const char* threadName) {
    SetThreadName(static_cast<std::uint32_t>(-1), threadName);
}

C4JThread::Event::Event(EMode mode)
    : m_mode(mode), m_mutex(), m_condition(), m_signaled(false) {}

void C4JThread::Event::Set() {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_signaled = true;
    }

    if (m_mode == e_modeAutoClear) {
        m_condition.notify_one();
    } else {
        m_condition.notify_all();
    }
}

void C4JThread::Event::Clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_signaled = false;
}

std::uint32_t C4JThread::Event::WaitForSignal(int timeoutMs) {
    std::unique_lock<std::mutex> lock(m_mutex);
    const bool signaled =
        WaitForCondition(m_condition, lock, timeoutMs,
                         [this] { return m_signaled; });

    if (!signaled) {
        return WAIT_TIMEOUT;
    }

    if (m_mode == e_modeAutoClear) {
        m_signaled = false;
    }

    return WAIT_OBJECT_0;
}

C4JThread::EventArray::EventArray(int size, EMode mode)
    : m_size(size),
      m_mode(mode),
      m_mutex(),
      m_condition(),
      m_signaledMask(0U) {
    assert(m_size > 0);
    assert(m_size <= 32);
}

void C4JThread::EventArray::Set(int index) {
    assert(index >= 0);
    assert(index < m_size);

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_signaledMask |= (1U << static_cast<std::uint32_t>(index));
    }

    m_condition.notify_all();
}

void C4JThread::EventArray::Clear(int index) {
    assert(index >= 0);
    assert(index < m_size);

    std::lock_guard<std::mutex> lock(m_mutex);
    m_signaledMask &= ~(1U << static_cast<std::uint32_t>(index));
}

void C4JThread::EventArray::SetAll() {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_signaledMask |= BuildMaskForSize(m_size);
    }

    m_condition.notify_all();
}

void C4JThread::EventArray::ClearAll() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_signaledMask = 0U;
}

std::uint32_t C4JThread::EventArray::WaitForSingle(int index, int timeoutMs) {
    assert(index >= 0);
    assert(index < m_size);

    const std::uint32_t bitMask = 1U << static_cast<std::uint32_t>(index);
    std::unique_lock<std::mutex> lock(m_mutex);

    const bool signaled =
        WaitForCondition(m_condition, lock, timeoutMs,
                         [this, bitMask] {
                             return (m_signaledMask & bitMask) != 0U;
                         });

    if (!signaled) {
        return WAIT_TIMEOUT;
    }

    if (m_mode == e_modeAutoClear) {
        m_signaledMask &= ~bitMask;
    }

    return WAIT_OBJECT_0;
}

std::uint32_t C4JThread::EventArray::WaitForAll(int timeoutMs) {
    const std::uint32_t bitMask = BuildMaskForSize(m_size);
    std::unique_lock<std::mutex> lock(m_mutex);

    const bool signaled =
        WaitForCondition(m_condition, lock, timeoutMs,
                         [this, bitMask] {
                             return (m_signaledMask & bitMask) == bitMask;
                         });

    if (!signaled) {
        return WAIT_TIMEOUT;
    }

    if (m_mode == e_modeAutoClear) {
        m_signaledMask &= ~bitMask;
    }

    return WAIT_OBJECT_0;
}

std::uint32_t C4JThread::EventArray::WaitForAny(int timeoutMs) {
    const std::uint32_t bitMask = BuildMaskForSize(m_size);
    std::unique_lock<std::mutex> lock(m_mutex);

    const bool signaled =
        WaitForCondition(m_condition, lock, timeoutMs,
                         [this, bitMask] {
                             return (m_signaledMask & bitMask) != 0U;
                         });

    if (!signaled) {
        return WAIT_TIMEOUT;
    }

    const std::uint32_t readyMask = m_signaledMask & bitMask;
    const std::uint32_t readyIndex = FirstSetBitIndex(readyMask);

    if (m_mode == e_modeAutoClear) {
        m_signaledMask &= ~(1U << readyIndex);
    }

    return WAIT_OBJECT_0 + readyIndex;
}

C4JThread::EventQueue::EventQueue(UpdateFunc* updateFunc,
                                  ThreadInitFunc* threadInitFunc,
                                  const char* threadName)
    : m_thread(),
      m_queue(),
      m_mutex(),
      m_queueCondition(),
      m_drainedCondition(),
      m_updateFunc(updateFunc),
      m_threadInitFunc(threadInitFunc),
      m_threadName(threadName != nullptr ? threadName : "Unnamed"),
      m_processor(-1),
      m_priority(kUnsetPriority),
      m_busy(false),
      m_initOnce(),
      m_stopRequested(false) {
    assert(m_updateFunc != nullptr);
}

C4JThread::EventQueue::~EventQueue() {
    m_stopRequested.store(true, std::memory_order_release);
    m_queueCondition.notify_all();

    if (m_thread) {
        (void)m_thread->WaitForCompletion(INFINITE);
    }
}

void C4JThread::EventQueue::setProcessor(int proc) {
    m_processor = proc;
    if (m_thread) {
        m_thread->SetProcessor(proc);
    }
}

void C4JThread::EventQueue::setPriority(int priority) {
    m_priority = priority;
    if (m_thread) {
        m_thread->SetPriority(priority);
    }
}

void C4JThread::EventQueue::init() {
    std::call_once(m_initOnce, [this]() {
        m_thread = std::make_unique<C4JThread>(threadFunc, this,
                                               m_threadName.c_str());

        if (m_processor >= 0) {
            m_thread->SetProcessor(m_processor);
        }

        if (m_priority != kUnsetPriority) {
            m_thread->SetPriority(m_priority);
        }

        m_thread->Run();
    });
}

void C4JThread::EventQueue::sendEvent(Level* pLevel) {
    init();

    if (m_stopRequested.load(std::memory_order_acquire)) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(pLevel);
    }

    m_queueCondition.notify_one();
}

void C4JThread::EventQueue::waitForFinish() {
    init();

    std::unique_lock<std::mutex> lock(m_mutex);
    m_drainedCondition.wait(lock, [this] {
        return m_queue.empty() && !m_busy;
    });
}

int C4JThread::EventQueue::threadFunc(void* lpParam) {
    EventQueue* pQueue = static_cast<EventQueue*>(lpParam);
    pQueue->threadPoll();
    return 0;
}

void C4JThread::EventQueue::threadPoll() {
    ShutdownManager::HasStarted(ShutdownManager::eEventQueueThreads);

    if (m_threadInitFunc != nullptr) {
        try {
            m_threadInitFunc();
        } catch (...) {
        }
    }

    while (!m_stopRequested.load(std::memory_order_acquire) &&
           ShutdownManager::ShouldRun(ShutdownManager::eEventQueueThreads)) {
        void* updateParam = nullptr;

        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_queueCondition.wait_for(lock,
                                      std::chrono::milliseconds(
                                          kEventQueueShutdownPollMs),
                                      [this] {
                                          return m_stopRequested.load(
                                                     std::memory_order_acquire) ||
                                                 !m_queue.empty();
                                      });

            if (m_stopRequested.load(std::memory_order_acquire)) {
                break;
            }

            if (m_queue.empty()) {
                continue;
            }

            m_busy = true;
            updateParam = m_queue.front();
            m_queue.pop();
        }

        try {
            m_updateFunc(updateParam);
        } catch (...) {
        }

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_busy = false;
            if (m_queue.empty()) {
                m_drainedCondition.notify_all();
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_busy = false;
        std::queue<void*> emptyQueue;
        m_queue.swap(emptyQueue);
    }
    m_drainedCondition.notify_all();

    ShutdownManager::HasFinished(ShutdownManager::eEventQueueThreads);
}

void C4JThread::PushAffinityAllCores() {
#if defined(_WIN32)
    const HANDLE currentThread = ::GetCurrentThread();
    DWORD_PTR processAffinityMask = 0;
    DWORD_PTR systemAffinityMask = 0;

    if (!::GetProcessAffinityMask(::GetCurrentProcess(),
                                  &processAffinityMask,
                                  &systemAffinityMask) ||
        processAffinityMask == 0) {
        return;
    }

    const DWORD_PTR previousMask =
        ::SetThreadAffinityMask(currentThread, processAffinityMask);

    if (previousMask != 0) {
        g_affinityMaskStack.push_back(previousMask);
    }

#elif defined(__linux__)
    cpu_set_t previousMask;
    if (::pthread_getaffinity_np(::pthread_self(), sizeof(previousMask),
                                 &previousMask) != 0) {
        return;
    }

    g_affinityMaskStack.push_back(previousMask);

    cpu_set_t allowedMask;
    if (::sched_getaffinity(0, sizeof(allowedMask), &allowedMask) != 0) {
        g_affinityMaskStack.pop_back();
        return;
    }

    (void)::pthread_setaffinity_np(::pthread_self(), sizeof(allowedMask),
                                   &allowedMask);
#endif
}

void C4JThread::PopAffinity() {
#if defined(_WIN32)
    if (g_affinityMaskStack.empty()) {
        return;
    }

    const DWORD_PTR previousMask = g_affinityMaskStack.back();
    g_affinityMaskStack.pop_back();
    (void)::SetThreadAffinityMask(::GetCurrentThread(), previousMask);

#elif defined(__linux__)
    if (g_affinityMaskStack.empty()) {
        return;
    }

    const cpu_set_t previousMask = g_affinityMaskStack.back();
    g_affinityMaskStack.pop_back();
    (void)::pthread_setaffinity_np(::pthread_self(), sizeof(previousMask),
                                   &previousMask);
#endif
}