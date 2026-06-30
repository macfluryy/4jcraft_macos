#include "Socket.h"

#include <stdio.h>

#include <chrono>
#include <thread>
#include <vector>

// 4jcraft TODO
#include "platform/ShutdownManager.h"
#include "app/common/src/Network/GameNetworkManager.h"
#include "app/common/src/Network/NetworkPlayerInterface.h"
#include "app/common/src/Network/RemoteNetworkPlayer.h"
#include "app/include/NetTypes.h"
#include "minecraft/server/network/ServerConnection.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <atomic>
#include <string>

class SocketAddress {};

// 4J macOS - ignore SIGPIPE process-wide exactly once. Writing to a TCP socket
// whose peer has already closed raises SIGPIPE, whose default disposition is to
// TERMINATE the process. That is precisely how the host was being killed when a
// remote client crashed/disconnected mid-stream (the shell showed "PIPE"). With
// SIGPIPE ignored, send() instead fails with errno==EPIPE which our send loop
// already handles gracefully (marks the end closed). This is the most portable
// fix (works on both macOS and Linux); we additionally set SO_NOSIGPIPE per
// socket on macOS and pass MSG_NOSIGNAL on send on Linux as belt-and-braces.
static void EnsureSigpipeIgnored() {
    static bool ignored = []() -> bool {
        signal(SIGPIPE, SIG_IGN);
        return true;
    }();
    (void)ignored;
}

// 4J macOS - portable "don't raise SIGPIPE for this fd". macOS has the
// SO_NOSIGPIPE socket option; Linux has no such option (it uses MSG_NOSIGNAL
// on send instead) so this is a no-op there. Safe to call on any TCP fd.
static void DisableSigpipeOnSocket(int fd) {
#if defined(SO_NOSIGPIPE)
    int one = 1;
    setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof(one));
#else
    (void)fd;
#endif
}

// 4J macOS - send flags that suppress SIGPIPE on platforms that support it at
// the call site (Linux MSG_NOSIGNAL). On macOS MSG_NOSIGNAL doesn't exist; the
// per-socket SO_NOSIGPIPE above covers it, so this resolves to 0.
#if defined(MSG_NOSIGNAL)
static const int kSendFlags = MSG_NOSIGNAL;
#else
static const int kSendFlags = 0;
#endif

// This current socket implementation is for the creation of a single local
// link. 2 sockets can be created, one for either end of this local link, the
// end (0 or 1) is passed as a parameter to the ctor.

std::mutex Socket::s_hostQueueLock[2];
std::queue<std::uint8_t> Socket::s_hostQueue[2];
Socket::SocketOutputStreamLocal* Socket::s_hostOutStream[2];
Socket::SocketInputStreamLocal* Socket::s_hostInStream[2];
ServerConnection* Socket::s_serverConnection = nullptr;

void Socket::EnsureStreamsInitialised() {
    // Thread-safe one-time initialisation via C++11 magic-statics guarantee.
    // The lambda body runs exactly once no matter how many threads call
    // concurrently.
    static bool initialized = []() -> bool {
        for (int i = 0; i < 2; i++) {
            s_hostOutStream[i] = new SocketOutputStreamLocal(i);
            s_hostInStream[i] = new SocketInputStreamLocal(i);
        }
        return true;
    }();
    (void)initialized;
}

void Socket::Initialise(ServerConnection* serverConnection) {
    s_serverConnection = serverConnection;

    // Ensure the host-local stream objects exist (idempotent).
    EnsureStreamsInitialised();

    // Only initialise everything else once - just setting up static data, one
    // time xrnm things, thread for ticking sockets
    static bool init = false;
    if (init) {
        // Streams already exist – just reset queue state and re-open streams.
        for (int i = 0; i < 2; i++) {
            {
                std::unique_lock<std::mutex> lock(s_hostQueueLock[i],
                                                  std::try_to_lock);
                if (lock.owns_lock()) {
                    // Clear the queue
                    std::queue<std::uint8_t> empty;
                    std::swap(s_hostQueue[i], empty);
                }
            }
            s_hostOutStream[i]->m_streamOpen = true;
            s_hostInStream[i]->m_streamOpen = true;
        }
        return;
    }
    init = true;
    // Streams are already guaranteed to exist via EnsureStreamsInitialised()
    // above. Nothing more to do for the first call.
}

Socket::Socket(bool response) {
    m_isTcp = false;
    m_tcpFd = -1;
    m_tcpReaderThread = nullptr;
    m_tcpRunning = false;

    m_hostServerConnection = true;
    m_hostLocal = true;
    if (response) {
        m_end = SOCKET_SERVER_END;
    } else {
        m_end = SOCKET_CLIENT_END;
        Socket* socket = new Socket(1);
        if (s_serverConnection != nullptr) {
            s_serverConnection->NewIncomingSocket(socket);
        } else {
            fprintf(
                stderr,
                "SOCKET: Warning - attempted to notify server of new incoming "
                "socket but s_serverConnection is nullptr\n");
        }
    }

    for (int i = 0; i < 2; i++) {
        m_endClosed[i] = false;
    }
    m_socketClosedEvent = nullptr;
    createdOk = true;
    networkPlayerSmallId = g_NetworkManager.GetHostPlayer()->GetSmallId();
}

Socket::Socket(INetworkPlayer* player, bool response /* = false*/,
               bool hostLocal /*= false*/) {
    m_isTcp = false;
    m_tcpFd = -1;
    m_tcpReaderThread = nullptr;
    m_tcpRunning = false;

    m_hostServerConnection = false;
    m_hostLocal = hostLocal;

    for (int i = 0; i < 2; i++) {
        m_inputStream[i] = nullptr;
        m_outputStream[i] = nullptr;
        m_endClosed[i] = false;
    }

    if (!response || hostLocal) {
        m_inputStream[0] = new SocketInputStreamNetwork(this, 0);
        m_outputStream[0] = new SocketOutputStreamNetwork(this, 0);
        m_end = SOCKET_CLIENT_END;
    }
    if (response || hostLocal) {
        m_inputStream[1] = new SocketInputStreamNetwork(this, 1);
        m_outputStream[1] = new SocketOutputStreamNetwork(this, 1);
        m_end = SOCKET_SERVER_END;
    }
    m_socketClosedEvent = new C4JThread::Event;
    // printf("New socket made %s\n", player->GetGamertag() );
    networkPlayerSmallId = player->GetSmallId();
    createdOk = true;
}
Socket::Socket(INetworkPlayer* player, int tcpFd, bool response) {
    m_hostServerConnection = false;
    m_hostLocal = false;
    m_isTcp = true;
    m_tcpFd = tcpFd;
    m_tcpReaderThread = nullptr;
    m_tcpRunning = false;

    for (int i = 0; i < 2; i++) {
        m_inputStream[i] = nullptr;
        m_outputStream[i] = nullptr;
        m_endClosed[i] = false;
    }

    if (response) {
        // Host / server-side socket.
        m_end = SOCKET_SERVER_END;
        m_inputStream[SOCKET_SERVER_END] =
            new SocketInputStreamNetwork(this, SOCKET_SERVER_END);
        m_outputStream[SOCKET_SERVER_END] =
            new SocketOutputStreamNetwork(this, SOCKET_SERVER_END);
    } else {
        // Client-side socket.
        m_end = SOCKET_CLIENT_END;
        m_inputStream[SOCKET_CLIENT_END] =
            new SocketInputStreamNetwork(this, SOCKET_CLIENT_END);
        m_outputStream[SOCKET_CLIENT_END] =
            new SocketOutputStreamNetwork(this, SOCKET_CLIENT_END);
    }

    m_socketClosedEvent = new C4JThread::Event;
    networkPlayerSmallId = (player != nullptr) ? player->GetSmallId() : 0;
    createdOk = true;

    // Disable Nagle so latency-sensitive small packets (keepalives, input)
    // are flushed immediately. Minecraft bursts a lot of tiny packets.
    int one = 1;
    setsockopt(m_tcpFd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));

    // 4J macOS - make sure a peer disconnect can never kill us with SIGPIPE.
    // Ignore it process-wide (idempotent) and, on macOS, also set the
    // per-socket SO_NOSIGPIPE option. Without this the host died with "PIPE"
    // the instant a remote client's socket closed mid-stream.
    EnsureSigpipeIgnored();
    DisableSigpipeOnSocket(m_tcpFd);

    m_tcpRunning = true;
    m_tcpReaderThread = new std::thread([this]() {
        // Reader loop - blocking recv on the TCP fd, push bytes into the
        // queue that matches this socket's end. Exits when recv returns
        // 0 / -1 or when m_tcpRunning is cleared.
        std::uint8_t buf[4096];
        while (m_tcpRunning.load()) {
            ssize_t n = recv(m_tcpFd, buf, sizeof(buf), 0);
            if (n <= 0) {
                if (n < 0 && (errno == EINTR || errno == EAGAIN)) continue;
                break;
            }
            {
                std::lock_guard<std::mutex> lock(m_queueLockNetwork[m_end]);
                for (ssize_t i = 0; i < n; ++i) {
                    m_queueNetwork[m_end].push(buf[i]);
                }
            }
            // 4J macOS - TCP recv throughput counter (per ~64 KB).
            m_tcpBytesRecv += (size_t)n;
            size_t total = m_tcpBytesRecv.load();
            size_t prev = m_tcpBytesRecvLastLog.load();
            if (total - prev >= 65536) {
                m_tcpBytesRecvLastLog.store(total);
                fprintf(stderr,
                        "[TCP] fd=%d recv total=%zu bytes (last=%zd)\n",
                        m_tcpFd, total, n);
            }
        }
        fprintf(stderr,
                "[TCP] reader exit fd=%d totalRecv=%zu\n", m_tcpFd,
                m_tcpBytesRecv.load());
        // Mark this end as closed so upper-layer reads bail out.
        m_endClosed[m_end] = true;
        if (m_socketClosedEvent != nullptr) m_socketClosedEvent->set();
    });
}

Socket::~Socket() {
    if (m_isTcp) {
        m_tcpRunning = false;
        if (m_tcpFd >= 0) {
            ::shutdown(m_tcpFd, SHUT_RDWR);
            ::close(m_tcpFd);
            m_tcpFd = -1;
        }
        if (m_tcpReaderThread != nullptr) {
            if (m_tcpReaderThread->joinable()) {
                m_tcpReaderThread->join();
            }
            delete m_tcpReaderThread;
            m_tcpReaderThread = nullptr;
        }
    }
}
namespace {
std::atomic<int> g_tcpListenerFd{-1};
std::atomic<int> g_tcpListenerPort{0};
std::atomic<bool> g_tcpListenerRunning{false};
std::thread* g_tcpAcceptThread = nullptr;
}  // namespace

bool Socket::IsTcpListenerRunning() { return g_tcpListenerRunning.load(); }
int Socket::GetTcpListenerPort() { return g_tcpListenerPort.load(); }

Socket* Socket::ConnectTcp(const std::string& host, int port,
                           INetworkPlayer* player) {
    // 4J macOS - install the process-wide SIGPIPE ignore before any socket
    // I/O can happen, so a peer disconnect never terminates the client.
    EnsureSigpipeIgnored();
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        fprintf(stderr, "[TCP] socket() failed: %s\n", strerror(errno));
        return nullptr;
    }

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    char portStr[16];
    snprintf(portStr, sizeof(portStr), "%d", port);

    struct addrinfo* result = nullptr;
    int err = getaddrinfo(host.c_str(), portStr, &hints, &result);
    if (err != 0 || result == nullptr) {
        fprintf(stderr, "[TCP] getaddrinfo(%s:%d) failed: %s\n", host.c_str(),
                port, gai_strerror(err));
        ::close(fd);
        return nullptr;
    }

    if (::connect(fd, result->ai_addr, result->ai_addrlen) != 0) {
        fprintf(stderr, "[TCP] connect(%s:%d) failed: %s\n", host.c_str(), port,
                strerror(errno));
        freeaddrinfo(result);
        ::close(fd);
        return nullptr;
    }
    freeaddrinfo(result);

    fprintf(stderr, "[TCP] Connected to %s:%d (fd=%d)\n", host.c_str(), port,
            fd);
    return new Socket(player, fd, /*response=*/false);
}

bool Socket::StartTcpListener(int port) {
    // 4J macOS - install the process-wide SIGPIPE ignore before any socket
    // I/O can happen, so a peer disconnect never terminates the host.
    EnsureSigpipeIgnored();
    if (g_tcpListenerRunning.load()) {
        fprintf(stderr, "[TCP] Listener already running on port %d\n",
                g_tcpListenerPort.load());
        return true;
    }

    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        fprintf(stderr, "[TCP] listener socket() failed: %s\n", strerror(errno));
        return false;
    }
    int yes = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((uint16_t)port);
    if (::bind(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        fprintf(stderr, "[TCP] bind(%d) failed: %s\n", port, strerror(errno));
        ::close(fd);
        return false;
    }
    if (::listen(fd, 8) != 0) {
        fprintf(stderr, "[TCP] listen() failed: %s\n", strerror(errno));
        ::close(fd);
        return false;
    }

    g_tcpListenerFd = fd;
    g_tcpListenerPort = port;
    g_tcpListenerRunning = true;

    g_tcpAcceptThread = new std::thread([]() {
        fprintf(stderr, "[TCP] Listener accepting on port %d\n",
                g_tcpListenerPort.load());
        while (g_tcpListenerRunning.load()) {
            struct sockaddr_in peer;
            socklen_t peerLen = sizeof(peer);
            int clientFd = ::accept(g_tcpListenerFd.load(),
                                    (struct sockaddr*)&peer, &peerLen);
            if (clientFd < 0) {
                if (!g_tcpListenerRunning.load()) break;
                if (errno == EINTR) continue;
                fprintf(stderr, "[TCP] accept() failed: %s\n", strerror(errno));
                break;
            }

            char ipbuf[INET_ADDRSTRLEN] = {0};
            inet_ntop(AF_INET, &peer.sin_addr, ipbuf, sizeof(ipbuf));
            fprintf(stderr,
                    "[TCP] Incoming connection from %s:%d (fd=%d)\n", ipbuf,
                    ntohs(peer.sin_port), clientFd);

            // 4J - wrap the per-client setup in try/catch so that a single
            // misbehaving connect (e.g. allocation failure, throw inside
            // CreateForIncoming) cannot terminate the listener thread and
            // take the whole server with it. Also explicitly close clientFd
            // on any local failure so we don't leak descriptors.
            try {
                // Create a remote-player stub for this connection and wire it
                // through the existing server machinery. The small-id is
                // handed out by RemoteNetworkPlayer::AllocateSmallId.
                RemoteNetworkPlayer* remote =
                    RemoteNetworkPlayer::CreateForIncoming(
                        ipbuf, ntohs(peer.sin_port));
                if (remote == nullptr) {
                    fprintf(stderr,
                            "[TCP] CreateForIncoming returned null, "
                            "dropping fd=%d\n",
                            clientFd);
                    ::close(clientFd);
                    continue;
                }

                Socket* serverSock =
                    new Socket(remote, clientFd, /*response=*/true);
                remote->SetSocket(serverSock);

                // Let the CGameNetworkManager know a player has joined so
                // the rest of the session bookkeeping (player lists, events)
                // fires.
                g_NetworkManager.DirectConnectPlayerJoining(remote);

                // Hand off to ServerConnection so the existing
                // PendingConnection flow picks up the handshake packets.
                Socket::addIncomingSocket(serverSock);
            } catch (const std::exception& e) {
                fprintf(stderr,
                        "[TCP] Exception while accepting fd=%d: %s\n",
                        clientFd, e.what());
                ::close(clientFd);
            } catch (...) {
                fprintf(stderr,
                        "[TCP] Unknown exception while accepting fd=%d, "
                        "dropping connection.\n",
                        clientFd);
                ::close(clientFd);
            }
        }
        fprintf(stderr, "[TCP] Listener thread exiting.\n");
    });

    return true;
}

void Socket::StopTcpListener() {
    if (!g_tcpListenerRunning.load()) return;
    g_tcpListenerRunning = false;
    int fd = g_tcpListenerFd.exchange(-1);
    if (fd >= 0) {
        ::shutdown(fd, SHUT_RDWR);
        ::close(fd);
    }
    if (g_tcpAcceptThread != nullptr) {
        if (g_tcpAcceptThread->joinable()) g_tcpAcceptThread->join();
        delete g_tcpAcceptThread;
        g_tcpAcceptThread = nullptr;
    }
}

SocketAddress* Socket::getRemoteSocketAddress() { return nullptr; }

INetworkPlayer* Socket::getPlayer() {
    return g_NetworkManager.GetPlayerBySmallId(networkPlayerSmallId);
}

void Socket::setPlayer(INetworkPlayer* player) {
    if (player != nullptr) {
        networkPlayerSmallId = player->GetSmallId();
    } else {
        networkPlayerSmallId = 0;
    }
}

void Socket::pushDataToQueue(const std::uint8_t* pbData, std::size_t dataSize,
                             bool fromHost /*= true*/) {
    int queueIdx = SOCKET_CLIENT_END;
    if (!fromHost) queueIdx = SOCKET_SERVER_END;

    if (queueIdx != m_end && !m_hostLocal) {
        fprintf(
            stderr,
            "SOCKET: Error pushing data to queue. End is %d but queue idx id "
            "%d\n",
            m_end, queueIdx);
        return;
    }

    {
        std::lock_guard<std::mutex> lock(m_queueLockNetwork[queueIdx]);
        for (std::size_t i = 0; i < dataSize; ++i) {
            m_queueNetwork[queueIdx].push(*pbData++);
        }
    }
}

void Socket::addIncomingSocket(Socket* socket) {
    if (s_serverConnection != nullptr) {
        s_serverConnection->NewIncomingSocket(socket);
    }
}

InputStream* Socket::getInputStream(bool isServerConnection) {
    if (!m_hostServerConnection) {
        if (m_hostLocal) {
            if (isServerConnection) {
                return m_inputStream[SOCKET_SERVER_END];
            } else {
                return m_inputStream[SOCKET_CLIENT_END];
            }
        } else {
            return m_inputStream[m_end];
        }
    } else {
        if (s_hostInStream[m_end] == nullptr) {
            fprintf(stderr,
                    "SOCKET: Warning - s_hostInStream[%d] is nullptr in "
                    "getInputStream(); calling EnsureStreamsInitialised()\n",
                    m_end);
            EnsureStreamsInitialised();
        }
        return s_hostInStream[m_end];
    }
}

void Socket::setSoTimeout(int a) {}

void Socket::setTrafficClass(int a) {}

Socket::SocketOutputStream* Socket::getOutputStream(bool isServerConnection) {
    if (!m_hostServerConnection) {
        if (m_hostLocal) {
            if (isServerConnection) {
                return m_outputStream[SOCKET_SERVER_END];
            } else {
                return m_outputStream[SOCKET_CLIENT_END];
            }
        } else {
            return m_outputStream[m_end];
        }
    } else {
        int outIdx = 1 - m_end;
        if (s_hostOutStream[outIdx] == nullptr) {
            fprintf(stderr,
                    "SOCKET: Warning - s_hostOutStream[%d] is nullptr in "
                    "getOutputStream(); calling EnsureStreamsInitialised()\n",
                    outIdx);
            EnsureStreamsInitialised();
        }
        return s_hostOutStream[outIdx];
    }
}

bool Socket::close(bool isServerConnection) {
    bool allClosed = false;
    if (m_hostLocal) {
        if (isServerConnection) {
            m_endClosed[SOCKET_SERVER_END] = true;
            if (m_endClosed[SOCKET_CLIENT_END]) {
                allClosed = true;
            }
        } else {
            m_endClosed[SOCKET_CLIENT_END] = true;
            if (m_endClosed[SOCKET_SERVER_END]) {
                allClosed = true;
            }
        }
    } else {
        allClosed = true;
        m_endClosed[m_end] = true;
        // 4J macOS - tear down the TCP fd so the reader thread unblocks and
        // remote sends start failing immediately. The std::thread object and
        // memory are cleaned up in ~Socket.
        if (m_isTcp) {
            m_tcpRunning = false;
            if (m_tcpFd >= 0) {
                ::shutdown(m_tcpFd, SHUT_RDWR);
            }
        }
    }
    if (allClosed && m_socketClosedEvent != nullptr) {
        m_socketClosedEvent->set();
    }
    if (allClosed) createdOk = false;
    return allClosed;
}

/////////////////////////////////// Socket for input, on local connection
///////////////////////

Socket::SocketInputStreamLocal::SocketInputStreamLocal(int queueIdx) {
    m_streamOpen = true;
    m_queueIdx = queueIdx;
}

// Try and get an input byte, blocking until one is available
int Socket::SocketInputStreamLocal::read() {
    while (m_streamOpen && ShutdownManager::ShouldRun(
                               ShutdownManager::eConnectionReadThreads)) {
        {
            std::unique_lock<std::mutex> lock(s_hostQueueLock[m_queueIdx],
                                              std::try_to_lock);
            if (lock.owns_lock()) {
                if (s_hostQueue[m_queueIdx].size()) {
                    std::uint8_t retval = s_hostQueue[m_queueIdx].front();
                    s_hostQueue[m_queueIdx].pop();
                    return retval;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return -1;
}

// Try and get an input array of bytes, blocking until enough bytes are
// available
int Socket::SocketInputStreamLocal::read(std::vector<uint8_t>& b) {
    return read(b, 0, b.size());
}

// Try and get an input range of bytes, blocking until enough bytes are
// available
int Socket::SocketInputStreamLocal::read(std::vector<uint8_t>& b,
                                         unsigned int offset,
                                         unsigned int length) {
    while (m_streamOpen) {
        {
            std::unique_lock<std::mutex> lock(s_hostQueueLock[m_queueIdx],
                                              std::try_to_lock);
            if (lock.owns_lock()) {
                if (s_hostQueue[m_queueIdx].size() >= length) {
                    for (unsigned int i = 0; i < length; i++) {
                        b[i + offset] = s_hostQueue[m_queueIdx].front();
                        s_hostQueue[m_queueIdx].pop();
                    }
                    return length;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return -1;
}

void Socket::SocketInputStreamLocal::close() {
    m_streamOpen = false;
    {
        std::lock_guard<std::mutex> lock(s_hostQueueLock[m_queueIdx]);
        std::queue<std::uint8_t>().swap(s_hostQueue[m_queueIdx]);
    }
}

/////////////////////////////////// Socket for output, on local connection
///////////////////////

Socket::SocketOutputStreamLocal::SocketOutputStreamLocal(int queueIdx) {
    m_streamOpen = true;
    m_queueIdx = queueIdx;
}

void Socket::SocketOutputStreamLocal::write(unsigned int b) {
    if (m_streamOpen != true) {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(s_hostQueueLock[m_queueIdx]);
        s_hostQueue[m_queueIdx].push((std::uint8_t)b);
    }
}

void Socket::SocketOutputStreamLocal::write(const std::vector<uint8_t>& b) {
    write(b, 0, b.size());
}

void Socket::SocketOutputStreamLocal::write(const std::vector<uint8_t>& b,
                                            unsigned int offset,
                                            unsigned int length) {
    if (m_streamOpen != true) {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(s_hostQueueLock[m_queueIdx]);
        for (unsigned int i = 0; i < length; i++) {
            s_hostQueue[m_queueIdx].push(b[offset + i]);
        }
    }
}

void Socket::SocketOutputStreamLocal::close() {
    m_streamOpen = false;
    {
        std::lock_guard<std::mutex> lock(s_hostQueueLock[m_queueIdx]);
        std::queue<std::uint8_t>().swap(s_hostQueue[m_queueIdx]);
    }
}

/////////////////////////////////// Socket for input, on network connection
///////////////////////

Socket::SocketInputStreamNetwork::SocketInputStreamNetwork(Socket* socket,
                                                           int queueIdx) {
    m_streamOpen = true;
    m_queueIdx = queueIdx;
    m_socket = socket;
}

// Try and get an input byte, blocking until one is available
int Socket::SocketInputStreamNetwork::read() {
    while (m_streamOpen && ShutdownManager::ShouldRun(
                               ShutdownManager::eConnectionReadThreads)) {
        {
            std::unique_lock<std::mutex> lock(
                m_socket->m_queueLockNetwork[m_queueIdx], std::try_to_lock);
            if (lock.owns_lock()) {
                if (m_socket->m_queueNetwork[m_queueIdx].size()) {
                    std::uint8_t retval =
                        m_socket->m_queueNetwork[m_queueIdx].front();
                    m_socket->m_queueNetwork[m_queueIdx].pop();
                    return retval;
                }
                // 4J macOS - TCP path: reader thread sets m_endClosed[end]
                // when the fd is torn down. No more bytes will ever arrive,
                // so stop blocking and report EOF to the upper layer.
                if (m_socket->m_isTcp && m_socket->m_endClosed[m_queueIdx]) {
                    return -1;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return -1;
}

// Try and get an input array of bytes, blocking until enough bytes are
// available
int Socket::SocketInputStreamNetwork::read(std::vector<uint8_t>& b) {
    return read(b, 0, b.size());
}

// Try and get an input range of bytes, blocking until enough bytes are
// available
int Socket::SocketInputStreamNetwork::read(std::vector<uint8_t>& b,
                                           unsigned int offset,
                                           unsigned int length) {
    while (m_streamOpen) {
        {
            std::unique_lock<std::mutex> lock(
                m_socket->m_queueLockNetwork[m_queueIdx], std::try_to_lock);
            if (lock.owns_lock()) {
                if (m_socket->m_queueNetwork[m_queueIdx].size() >= length) {
                    for (unsigned int i = 0; i < length; i++) {
                        b[i + offset] =
                            m_socket->m_queueNetwork[m_queueIdx].front();
                        m_socket->m_queueNetwork[m_queueIdx].pop();
                    }
                    return length;
                }
                // 4J macOS - TCP path: if the remote has gone away and there
                // aren't enough bytes left in the queue to satisfy the read,
                // return EOF instead of spinning forever.
                if (m_socket->m_isTcp && m_socket->m_endClosed[m_queueIdx] &&
                    m_socket->m_queueNetwork[m_queueIdx].size() < length) {
                    return -1;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return -1;
}

void Socket::SocketInputStreamNetwork::close() { m_streamOpen = false; }

/////////////////////////////////// Socket for output, on network connection
///////////////////////

Socket::SocketOutputStreamNetwork::SocketOutputStreamNetwork(Socket* socket,
                                                             int queueIdx) {
    m_queueIdx = queueIdx;
    m_socket = socket;
    m_streamOpen = true;
}

void Socket::SocketOutputStreamNetwork::write(unsigned int b) {
    if (m_streamOpen != true) return;
    std::uint8_t bb = (std::uint8_t)b;
    std::vector<uint8_t> barray(1, bb);
    write(barray, 0, 1);
}

void Socket::SocketOutputStreamNetwork::write(const std::vector<uint8_t>& b) {
    write(b, 0, b.size());
}

void Socket::SocketOutputStreamNetwork::write(const std::vector<uint8_t>& b,
                                              unsigned int offset,
                                              unsigned int length) {
    writeWithFlags(b, offset, length, 0);
}

void Socket::SocketOutputStreamNetwork::writeWithFlags(
    const std::vector<uint8_t>& b, unsigned int offset, unsigned int length,
    int flags) {
    if (m_streamOpen != true) return;
    if (length == 0) return;
    if (m_socket->m_isTcp) {
        std::lock_guard<std::mutex> lock(m_socket->m_tcpWriteMutex);
        int fd = m_socket->m_tcpFd;
        if (fd < 0) return;
        const uint8_t* p = &b[offset];
        size_t remaining = length;
        while (remaining > 0) {
            ssize_t sent = ::send(fd, p, remaining, kSendFlags);
            if (sent <= 0) {
                if (sent < 0 && (errno == EINTR || errno == EAGAIN)) continue;
                fprintf(stderr, "[TCP] send() failed on fd=%d: %s\n", fd,
                        strerror(errno));
                m_socket->m_endClosed[m_socket->m_end] = true;
                if (m_socket->m_socketClosedEvent != nullptr)
                    m_socket->m_socketClosedEvent->set();
                return;
            }
            p += sent;
            remaining -= (size_t)sent;
        }
        // 4J macOS - TCP send throughput counter (logged once per ~64 KB).
        m_socket->m_tcpBytesSent += length;
        size_t total = m_socket->m_tcpBytesSent.load();
        size_t prev = m_socket->m_tcpBytesSentLastLog.load();
        if (total - prev >= 65536) {
            m_socket->m_tcpBytesSentLastLog.store(total);
            fprintf(stderr,
                    "[TCP] fd=%d sent total=%zu bytes (last chunk=%u)\n", fd,
                    total, length);
        }
        return;
    }

    // If this is a local connection, don't bother going through QNet as it just
    // delivers it straight anyway
    if (m_socket->m_hostLocal) {
        // We want to write to the queue for the other end of this socket stream
        int queueIdx = m_queueIdx;
        if (queueIdx == SOCKET_CLIENT_END)
            queueIdx = SOCKET_SERVER_END;
        else
            queueIdx = SOCKET_CLIENT_END;

        {
            std::lock_guard<std::mutex> lock(
                m_socket->m_queueLockNetwork[queueIdx]);
            for (unsigned int i = 0; i < length; i++) {
                m_socket->m_queueNetwork[queueIdx].push(b[offset + i]);
            }
        }
    } else {
        XRNM_SEND_BUFFER buffer;
        buffer.pbyData = const_cast<uint8_t*>(&b[offset]);
        buffer.dwDataSize = length;

        INetworkPlayer* hostPlayer = g_NetworkManager.GetHostPlayer();
        if (hostPlayer == nullptr) {
            fprintf(
                stderr,
                "Trying to write to network, but the hostPlayer is nullptr\n");
            return;
        }
        INetworkPlayer* socketPlayer = m_socket->getPlayer();
        if (socketPlayer == nullptr) {
            fprintf(stderr,
                    "Trying to write to network, but the socketPlayer is "
                    "nullptr\n");
            return;
        }

        bool lowPriority = false;
        bool requireAck = ((flags & NON_QNET_SENDDATA_ACK_REQUIRED) ==
                           NON_QNET_SENDDATA_ACK_REQUIRED);

        if (m_queueIdx == SOCKET_SERVER_END) {
            // printf( "Sent %u bytes of data from \"%ls\" to \"%ls\"\n",
            // buffer.dwDataSize,
            // hostPlayer->GetGamertag(),
            // m_socket->networkPlayer->GetGamertag());

            hostPlayer->SendData(socketPlayer, buffer.pbyData,
                                 buffer.dwDataSize, lowPriority, requireAck);

            // 		uint32_t queueSize = hostPlayer->GetSendQueueSize(
            // nullptr, QNET_GETSENDQUEUESIZE_BYTES  ); 		if(
            // queueSize > 24000 )
            // 		{
            // 			//printf("Queue size is: %d, forcing
            // doWork()\n",queueSize); g_NetworkManager.DoWork();
            // 		}
        } else {
            // printf( "Sent %u bytes of data from \"%ls\" to \"%ls\"\n",
            // buffer.dwDataSize,
            // m_socket->networkPlayer->GetGamertag(),
            // hostPlayer->GetGamertag());

            socketPlayer->SendData(hostPlayer, buffer.pbyData,
                                   buffer.dwDataSize, lowPriority, requireAck);
        }
    }
}

void Socket::SocketOutputStreamNetwork::close() { m_streamOpen = false; }
