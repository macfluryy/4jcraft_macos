#pragma once
#include <cstddef>
#include <cstdint>
#include <format>
#include <vector>
#if !defined(__linux__) && !defined(__APPLE__)
#include <qnet.h>
#include <xrnm.h>
#endif
#include <atomic>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include "app/common/src/Network/GameNetworkManager.h"
#include "app/common/src/Network/NetworkPlayerInterface.h"
#include "platform/C4JThread.h"
#include "java/InputOutputStream/InputStream.h"
#include "java/InputOutputStream/OutputStream.h"

class INetworkPlayer;

#define SOCKET_CLIENT_END 0
#define SOCKET_SERVER_END 1

class SocketAddress;
class ServerConnection;

class Socket {
public:
    class SocketOutputStream : public OutputStream {
    public:
        virtual void writeWithFlags(const std::vector<uint8_t>& b,
                                    unsigned int offset, unsigned int length,
                                    int flags) {
            write(b, offset, length);
        }
    };

private:
    class SocketInputStreamLocal : public InputStream {
    public:
        bool m_streamOpen;

    private:
        int m_queueIdx;

    public:
        SocketInputStreamLocal(int queueIdx);

        virtual int read();
        virtual int read(std::vector<uint8_t>& b);
        virtual int read(std::vector<uint8_t>& b, unsigned int offset,
                         unsigned int length);
        virtual void close();
        virtual int64_t skip(int64_t n) {
            return n;
        }  
        virtual void flush() {}
    };

    class SocketOutputStreamLocal : public SocketOutputStream {
    public:
        bool m_streamOpen;

    private:
        int m_queueIdx;

    public:
        SocketOutputStreamLocal(int queueIdx);

        virtual void write(unsigned int b);
        virtual void write(const std::vector<uint8_t>& b);
        virtual void write(const std::vector<uint8_t>& b, unsigned int offset,
                           unsigned int length);
        virtual void close();
        virtual void flush() {}
    };

    class SocketInputStreamNetwork : public InputStream {
        bool m_streamOpen;
        int m_queueIdx;
        Socket* m_socket;

    public:
        SocketInputStreamNetwork(Socket* socket, int queueIdx);

        virtual int read();
        virtual int read(std::vector<uint8_t>& b);
        virtual int read(std::vector<uint8_t>& b, unsigned int offset,
                         unsigned int length);
        virtual void close();
        virtual int64_t skip(int64_t n) {
            return n;
        }  
        virtual void flush() {}
    };
    class SocketOutputStreamNetwork : public SocketOutputStream {
        bool m_streamOpen;
        int m_queueIdx;
        Socket* m_socket;

    public:
        SocketOutputStreamNetwork(Socket* socket, int queueIdx);

        virtual void write(unsigned int b);
        virtual void write(const std::vector<uint8_t>& b);
        virtual void write(const std::vector<uint8_t>& b, unsigned int offset,
                           unsigned int length);
        virtual void writeWithFlags(const std::vector<uint8_t>& b,
                                    unsigned int offset, unsigned int length,
                                    int flags);
        virtual void close();
        virtual void flush() {}
    };

    bool m_hostServerConnection;
    bool m_hostLocal;
    int m_end;

    static std::mutex s_hostQueueLock[2];
    static std::queue<std::uint8_t> s_hostQueue[2];
    static SocketOutputStreamLocal* s_hostOutStream[2];
    static SocketInputStreamLocal* s_hostInStream[2];

    std::queue<std::uint8_t> m_queueNetwork[2];
    std::mutex m_queueLockNetwork[2];
    SocketInputStreamNetwork* m_inputStream[2];
    SocketOutputStreamNetwork* m_outputStream[2];
    std::atomic<bool> m_endClosed[2];
    bool m_isTcp;
    int m_tcpFd;
    std::thread* m_tcpReaderThread;
    std::atomic<bool> m_tcpRunning;
    std::mutex m_tcpWriteMutex;
    std::atomic<size_t> m_tcpBytesSent{0};
    std::atomic<size_t> m_tcpBytesSentLastLog{0};
    std::atomic<size_t> m_tcpBytesRecv{0};
    std::atomic<size_t> m_tcpBytesRecvLastLog{0};

    static ServerConnection* s_serverConnection;

    std::uint8_t networkPlayerSmallId;

public:
    C4JThread::Event* m_socketClosedEvent;

    INetworkPlayer* getPlayer();
    void setPlayer(INetworkPlayer* player);

public:
    static void
    EnsureStreamsInitialised();
    static void Initialise(ServerConnection* serverConnection);
    Socket(bool response = false);
    Socket(
        INetworkPlayer* player, bool response = false,
        bool hostLocal = false);
    Socket(INetworkPlayer* player, int tcpFd, bool response);
    static Socket* ConnectTcp(const std::string& host, int port,
                              INetworkPlayer* player);
    static bool StartTcpListener(int port);
    static void StopTcpListener();
    static bool IsTcpListenerRunning();
    static int GetTcpListenerPort();

    SocketAddress* getRemoteSocketAddress();
    void pushDataToQueue(const std::uint8_t* pbData, std::size_t dataSize,
                         bool fromHost = true);
    static void addIncomingSocket(Socket* socket);
    InputStream* getInputStream(bool isServerConnection);
    void setSoTimeout(int a);
    void setTrafficClass(int a);
    SocketOutputStream* getOutputStream(bool isServerConnection);
    bool close(bool isServerConnection);
    ~Socket();
    bool createdOk;
    bool isLocal() { return m_hostLocal; }
    bool isTcp() { return m_isTcp; }
    int getTcpFd() { return m_tcpFd; }

    bool isClosing() {
        return m_endClosed[SOCKET_CLIENT_END] || m_endClosed[SOCKET_SERVER_END];
    }
    std::uint8_t getSmallId() { return networkPlayerSmallId; }
};