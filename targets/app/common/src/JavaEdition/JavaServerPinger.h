#pragma once
#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

enum class PingStatus {
    Querying,
    Online,
    Unreachable,
};

struct PingResult {
    PingStatus status = PingStatus::Querying;
    std::wstring motd;
    int online = -1;
    int max = -1;
    int pingMs = -1;
    std::wstring version;
};

class JavaServerPinger {
public:
    void startPing(int entryId, const std::string& host, uint16_t port);
    bool pollResult(int entryId, PingResult& out);
    void cancelAll();
    ~JavaServerPinger();

private:
    void runPing(int entryId, std::string host, uint16_t port);
    bool storeResult(int entryId, const PingResult& result);

    std::mutex m_mutex;
    std::unordered_map<int, PingResult> m_results;
    std::vector<std::thread> m_workers;
    std::atomic<bool> m_cancelled{false};
};