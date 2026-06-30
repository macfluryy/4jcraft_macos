#include "app/common/src/JavaEdition/JavaServerPinger.h"

#include <chrono>
#include <exception>
#include <functional>

#include "app/common/src/JavaEdition/JavaPacketBuffer.h"
#include "app/common/src/JavaEdition/JavaRawTcpClient.h"
#include "app/common/src/JavaEdition/StatusJsonExtractor.h"

namespace {

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::steady_clock;

constexpr int kConnectTimeoutMs = 5000;
constexpr int kReadTimeoutMs = 5000;
constexpr int32_t kProtocolVersion1_8 = 47;
constexpr int32_t kPacketIdHandshake = 0x00;
constexpr int32_t kPacketIdStatusRequest = 0x00;
constexpr int32_t kPacketIdStatusResponse = 0x00;
constexpr int32_t kPacketIdPing = 0x01;
constexpr int32_t kPacketIdPong = 0x01;
constexpr int32_t kNextStateStatus = 1;

void appendFramed(JavaPacketWriter& out, const std::vector<uint8_t>& body) {
    out.writeVarInt(static_cast<int32_t>(body.size()));
    out.writeBytes(body.data(), body.size());
}

std::vector<uint8_t> readFrame(JavaRawTcpClient& client, int timeoutMs) {
    const std::function<int()> nextByte = [&client, timeoutMs]() -> int {
        uint8_t b = 0;
        const size_t n = client.recv(&b, 1, timeoutMs);
        if (n == 0) return -1;
        return static_cast<int>(b);
    };

    const VarIntReadResult len = readVarIntFromStream(nextByte);
    if (!len.ok) {
        throw JavaProtocolError("JavaServerPinger: bad/truncated frame length");
    }
    if (len.value < 0 || static_cast<size_t>(len.value) > kMaxFrameLength) {
        throw JavaProtocolError("JavaServerPinger: frame length exceeds cap");
    }

    const size_t length = static_cast<size_t>(len.value);
    std::vector<uint8_t> frame(length);
    size_t got = 0;
    while (got < length) {
        const size_t n = client.recv(frame.data() + got, length - got, timeoutMs);
        if (n == 0) {
            throw JavaProtocolError("JavaServerPinger: frame body truncated");
        }
        got += n;
    }
    return frame;
}

}

void JavaServerPinger::startPing(int entryId, const std::string& host, uint16_t port) {
    std::lock_guard<std::mutex> lock(m_mutex);
    PingResult querying;
    querying.status = PingStatus::Querying;
    m_results[entryId] = querying;
    m_workers.emplace_back(&JavaServerPinger::runPing, this, entryId, host, port);
}

bool JavaServerPinger::pollResult(int entryId, PingResult& out) {
    std::lock_guard<std::mutex> lock(m_mutex);
    const auto it = m_results.find(entryId);
    if (it == m_results.end()) {
        return false;
    }
    out = it->second;
    return true;
}

bool JavaServerPinger::storeResult(int entryId, const PingResult& result) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_cancelled.load()) {
        return false;
    }
    m_results[entryId] = result;
    return true;
}

void JavaServerPinger::cancelAll() {
    m_cancelled.store(true);

    std::vector<std::thread> workers;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        workers.swap(m_workers);
    }
    for (std::thread& t : workers) {
        if (t.joinable()) {
            t.join();
        }
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    m_results.clear();
}

JavaServerPinger::~JavaServerPinger() {
    cancelAll();
}

void JavaServerPinger::runPing(int entryId, std::string host, uint16_t port) {
    PingResult result;
    result.status = PingStatus::Querying;

    try {
        if (m_cancelled.load()) return;

        JavaRawTcpClient client;
        if (!client.connect(host, port, kConnectTimeoutMs)) {
            result.status = PingStatus::Unreachable;
            storeResult(entryId, result);
            return;
        }

        if (m_cancelled.load()) return;

        JavaPacketWriter handshakeBody;
        handshakeBody.writeVarInt(kPacketIdHandshake);
        handshakeBody.writeVarInt(kProtocolVersion1_8);
        handshakeBody.writeUtf8(host);
        handshakeBody.writeU16(port);
        handshakeBody.writeVarInt(kNextStateStatus);

        JavaPacketWriter statusBody;
        statusBody.writeVarInt(kPacketIdStatusRequest);

        JavaPacketWriter out;
        appendFramed(out, handshakeBody.bytes());
        appendFramed(out, statusBody.bytes());

        const steady_clock::time_point statusStart = steady_clock::now();
        if (!client.sendAll(out.bytes().data(), out.bytes().size())) {
            result.status = PingStatus::Unreachable;
            storeResult(entryId, result);
            return;
        }

        const std::vector<uint8_t> frame = readFrame(client, kReadTimeoutMs);
        JavaPacketReader reader(frame.data(), frame.size());
        const int32_t packetId = reader.readVarInt();
        if (packetId != kPacketIdStatusResponse) {
            result.status = PingStatus::Unreachable;
            storeResult(entryId, result);
            return;
        }
        const std::string json = reader.readUtf8(kMaxStringBytes);
        const StatusInfo info = extractStatus(json);
        if (!info.valid) {
            result.status = PingStatus::Unreachable;
            storeResult(entryId, result);
            return;
        }
        result.motd = info.motd;
        result.online = info.online;
        result.max = info.max;
        result.version = info.version;

        const int fallbackMs = static_cast<int>(
            duration_cast<milliseconds>(steady_clock::now() - statusStart).count());

        if (m_cancelled.load()) return;
        int pingMs = -1;
        try {
            const steady_clock::time_point pingStart = steady_clock::now();
            const int64_t payload = static_cast<int64_t>(
                duration_cast<milliseconds>(pingStart.time_since_epoch()).count());

            JavaPacketWriter pingBody;
            pingBody.writeVarInt(kPacketIdPing);
            pingBody.writeI64(payload);
            JavaPacketWriter pingOut;
            appendFramed(pingOut, pingBody.bytes());

            if (client.sendAll(pingOut.bytes().data(), pingOut.bytes().size())) {
                const std::vector<uint8_t> pong = readFrame(client, kReadTimeoutMs);
                JavaPacketReader pongReader(pong.data(), pong.size());
                const int32_t pongId = pongReader.readVarInt();
                if (pongId == kPacketIdPong) {
                    pongReader.readI64();
                    pingMs = static_cast<int>(
                        duration_cast<milliseconds>(steady_clock::now() - pingStart).count());
                }
            }
        } catch (...) {
            pingMs = -1;
        }

        if (pingMs < 0) {
            pingMs = fallbackMs;
        }
        if (pingMs < 0) {
            pingMs = 0;
        }

        result.pingMs = pingMs;
        result.status = PingStatus::Online;
        storeResult(entryId, result);
    } catch (const std::exception&) {
        PingResult err;
        err.status = PingStatus::Unreachable;
        storeResult(entryId, err);
    } catch (...) {
        PingResult err;
        err.status = PingStatus::Unreachable;
        storeResult(entryId, err);
    }
}