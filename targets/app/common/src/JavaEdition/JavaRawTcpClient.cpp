#include "app/common/src/JavaEdition/JavaRawTcpClient.h"

#include <cerrno>
#include <cstring>

#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

namespace {

#if defined(MSG_NOSIGNAL)
constexpr int kSendFlags = MSG_NOSIGNAL;
#else
constexpr int kSendFlags = 0;
#endif

void disableSigpipeOnSocket(int fd) {
#if defined(SO_NOSIGPIPE)
    int one = 1;
    setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof(one));
#else
    (void)fd;
#endif
}

bool setNonBlocking(int fd, bool nonBlocking) {
    const int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return false;
    const int updated = nonBlocking ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
    return fcntl(fd, F_SETFL, updated) >= 0;
}

void applySocketTimeout(int fd, int optName, int timeoutMs) {
    if (timeoutMs < 1) timeoutMs = 1;
    struct timeval tv;
    tv.tv_sec = timeoutMs / 1000;
    tv.tv_usec = (timeoutMs % 1000) * 1000;
    setsockopt(fd, SOL_SOCKET, optName, &tv, sizeof(tv));
}

}

void JavaRawTcpClient::ensureSigpipeIgnored() {
    static bool ignored = []() -> bool {
        signal(SIGPIPE, SIG_IGN);
        return true;
    }();
    (void)ignored;
}

bool JavaRawTcpClient::connect(const std::string& host, uint16_t port, int timeoutMs) {
    close();

    ensureSigpipeIgnored();

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    char portStr[16];
    snprintf(portStr, sizeof(portStr), "%u", static_cast<unsigned>(port));

    struct addrinfo* result = nullptr;
    const int err = getaddrinfo(host.c_str(), portStr, &hints, &result);
    if (err != 0 || result == nullptr) {
        return false;
    }

    if (timeoutMs < 0) timeoutMs = 0;
    const int64_t deadlineMs = static_cast<int64_t>(timeoutMs);
    struct timeval start;
    gettimeofday(&start, nullptr);

    int connectedFd = -1;
    for (struct addrinfo* ai = result; ai != nullptr; ai = ai->ai_next) {
        const int fd = ::socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (fd < 0) continue;

        if (!setNonBlocking(fd, true)) {
            ::close(fd);
            continue;
        }

        int rc = ::connect(fd, ai->ai_addr, ai->ai_addrlen);
        if (rc == 0) {
            connectedFd = fd;
            break;
        }
        if (errno != EINPROGRESS) {
            ::close(fd);
            continue;
        }

        struct timeval now;
        gettimeofday(&now, nullptr);
        const int64_t elapsedMs = (static_cast<int64_t>(now.tv_sec - start.tv_sec) * 1000) +
                                  ((now.tv_usec - start.tv_usec) / 1000);
        int remainingMs = static_cast<int>(deadlineMs - elapsedMs);
        if (remainingMs < 0) remainingMs = 0;

        struct pollfd pfd;
        pfd.fd = fd;
        pfd.events = POLLOUT;
        pfd.revents = 0;

        int pr;
        do {
            pr = poll(&pfd, 1, remainingMs);
        } while (pr < 0 && errno == EINTR);

        if (pr <= 0) {
            ::close(fd);
            continue;
        }

        int soError = 0;
        socklen_t soLen = sizeof(soError);
        if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &soError, &soLen) < 0 || soError != 0) {
            ::close(fd);
            continue;
        }

        connectedFd = fd;
        break;
    }

    freeaddrinfo(result);

    if (connectedFd < 0) {
        return false;
    }

    if (!setNonBlocking(connectedFd, false)) {
        ::close(connectedFd);
        return false;
    }
    int rcvTimeout = timeoutMs > 0 ? timeoutMs : 5000;
    applySocketTimeout(connectedFd, SO_RCVTIMEO, rcvTimeout);
    applySocketTimeout(connectedFd, SO_SNDTIMEO, rcvTimeout);
    disableSigpipeOnSocket(connectedFd);

    int one = 1;
    setsockopt(connectedFd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));

    m_fd = connectedFd;
    return true;
}

bool JavaRawTcpClient::sendAll(const uint8_t* data, size_t len) {
    if (len == 0) return true;
    if (m_fd < 0 || data == nullptr) return false;

    size_t sent = 0;
    while (sent < len) {
        const ssize_t n = ::send(m_fd, data + sent, len - sent, kSendFlags);
        if (n > 0) {
            sent += static_cast<size_t>(n);
            continue;
        }
        if (n < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)) {
            continue;
        }
        return false;
    }
    return true;
}

size_t JavaRawTcpClient::recv(uint8_t* out, size_t len, int timeoutMs) {
    if (m_fd < 0 || out == nullptr || len == 0) return 0;

    struct pollfd pfd;
    pfd.fd = m_fd;
    pfd.events = POLLIN;
    pfd.revents = 0;

    int pr;
    do {
        pr = poll(&pfd, 1, timeoutMs < 0 ? 0 : timeoutMs);
    } while (pr < 0 && errno == EINTR);

    if (pr <= 0) {
        return 0;
    }

    ssize_t n;
    do {
        n = ::recv(m_fd, out, len, 0);
    } while (n < 0 && errno == EINTR);

    if (n <= 0) {
        return 0;
    }
    return static_cast<size_t>(n);
}

void JavaRawTcpClient::close() {
    if (m_fd >= 0) {
        ::close(m_fd);
        m_fd = -1;
    }
}

JavaRawTcpClient::~JavaRawTcpClient() {
    close();
}