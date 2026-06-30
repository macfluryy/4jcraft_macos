#pragma once
#include <cstddef>
#include <cstdint>
#include <string>

class JavaRawTcpClient {
public:
    bool connect(const std::string& host, uint16_t port, int timeoutMs);
    bool sendAll(const uint8_t* data, size_t len);

    size_t recv(uint8_t* out, size_t len, int timeoutMs);
    void close();
    ~JavaRawTcpClient();

private:
    int m_fd = -1;
    static void ensureSigpipeIgnored();
};