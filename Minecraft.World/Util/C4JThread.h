#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <limits>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

using C4JThreadStartFunc = int(void* lpThreadParameter);

class Level;

inline constexpr int CPU_CORE_MAIN_THREAD = 0;

inline constexpr int CPU_CORE_CHUNK_REBUILD_A = 1;
inline constexpr int CPU_CORE_SAVE_THREAD_A = 1;
inline constexpr int CPU_CORE_TILE_UPDATE = 1;
inline constexpr int CPU_CORE_CONNECTIONS = 1;

inline constexpr int CPU_CORE_CHUNK_UPDATE = 2;
inline constexpr int CPU_CORE_REMOVE_PLAYER = 2;

inline constexpr int CPU_CORE_CHUNK_REBUILD_B = 3;
inline constexpr int CPU_CORE_SAVE_THREAD_B = 3;
inline constexpr int CPU_CORE_UI_SCENE = 3;
inline constexpr int CPU_CORE_POST_PROCESSING = 3;

inline constexpr int CPU_CORE_SERVER = 4;

inline constexpr int CPU_CORE_CHUNK_REBUILD_C = 5;
inline constexpr int CPU_CORE_SAVE_THREAD_C = 5;
inline constexpr int CPU_CORE_LEADERBOARDS = 5;

class C4JThread {
public:
    class Event {
    public:
        enum EMode { e_modeAutoClear, e_modeManualClear };

        explicit Event(EMode mode = e_modeAutoClear);
        ~Event() = default;

        void Set();
        void Clear();
        std::uint32_t WaitForSignal(int timeoutMs);

    private:
        EMode m_mode;
        std::mutex m_mutex;
        std::condition_variable m_condition;
        bool m_signaled;
    };

    class EventArray {
    public:
        enum EMode { e_modeAutoClear, e_modeManualClear };

        explicit EventArray(int size, EMode mode = e_modeAutoClear);

        void Set(int index);
        void Clear(int index);
        void SetAll();
        void ClearAll();
        std::uint32_t WaitForAll(int timeoutMs);
        std::uint32_t WaitForAny(int timeoutMs);
        std::uint32_t WaitForSingle(int index, int timeoutMs);

    private:
        int m_size;
        EMode m_mode;
        std::mutex m_mutex;
        std::condition_variable m_condition;
        std::uint32_t m_signaledMask;
    };

    class EventQueue {
    public:
        using UpdateFunc = void(void* lpParameter);
        using ThreadInitFunc = void();

        EventQueue(UpdateFunc* updateFunc, ThreadInitFunc* threadInitFunc,
                   const char* threadName);
        ~EventQueue();

        EventQueue(const EventQueue&) = delete;
        EventQueue& operator=(const EventQueue&) = delete;

        void setProcessor(int proc);
        void setPriority(int priority);
        void sendEvent(Level* pLevel);
        void waitForFinish();

    private:
        void init();
        static int threadFunc(void* lpParam);
        void threadPoll();

        std::unique_ptr<C4JThread> m_thread;
        std::queue<void*> m_queue;
        std::mutex m_mutex;
        std::condition_variable m_queueCondition;
        std::condition_variable m_drainedCondition;
        UpdateFunc* m_updateFunc;
        ThreadInitFunc* m_threadInitFunc;
        std::string m_threadName;
        int m_processor;
        int m_priority;
        bool m_busy;
        std::once_flag m_initOnce;
        std::atomic<bool> m_stopRequested;
    };

    C4JThread(C4JThreadStartFunc* startFunc, void* param,
              const char* threadName, int stackSize = 0);
    explicit C4JThread(const char* mainThreadName);
    ~C4JThread();

    C4JThread(const C4JThread&) = delete;
    C4JThread& operator=(const C4JThread&) = delete;

    void Run();

    [[nodiscard]] bool isRunning() const noexcept { return m_isRunning.load(); }
    [[nodiscard]] bool hasStarted() const noexcept { return m_hasStarted.load(); }

    void SetProcessor(int proc);
    void SetPriority(int priority);

    std::uint32_t WaitForCompletion(int timeoutMs);
    [[nodiscard]] int GetExitCode() const noexcept;

    [[nodiscard]] const char* getName() const noexcept {
        return m_threadName.c_str();
    }

    static void Sleep(int millisecs);
    static C4JThread* getCurrentThread() noexcept;
    static bool isMainThread() noexcept;

    static const char* getCurrentThreadName() noexcept {
        const C4JThread* pThread = getCurrentThread();
        return pThread ? pThread->getName() : "(4J) Unknown thread";
    }

    static void SetThreadName(std::uint32_t threadId, const char* threadName);
    static void SetCurrentThreadName(const char* threadName);

    static void PushAffinityAllCores();
    static void PopAffinity();

private:
    static void entryPoint(C4JThread* pThread);
    static C4JThread& getMainThreadInstance() noexcept;

    void* m_threadParam;
    C4JThreadStartFunc* m_startFunc;
    int m_stackSize;
    std::string m_threadName;
    std::atomic<bool> m_isRunning;
    std::atomic<bool> m_hasStarted;
    std::atomic<int> m_exitCode;

    std::thread::id m_threadID;
    std::thread m_threadHandle;
    std::unique_ptr<Event> m_completionFlag;

    std::atomic<int> m_requestedProcessor;
    std::atomic<int> m_requestedPriority;
    std::atomic<std::int64_t> m_nativeTid;

    static thread_local C4JThread* ms_currentThread;
};