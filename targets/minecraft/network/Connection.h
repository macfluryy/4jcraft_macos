#pragma once

#include <stdint.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

#include "app/common/src/Network/Socket.h"
#include "app/mac/Stubs/winapi_stubs.h"
#include "platform/C4JThread.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "java/InputOutputStream/DataOutputStream.h"
#include "java/System.h"
#include "minecraft/network/packet/DisconnectPacket.h"


class DataInputStream;
class DataOutputStream;
class Packet;
class PacketListener;
class SocketAddress;


#define READ_STACK_SIZE 0
#define WRITE_STACK_SIZE 0

class ByteArrayOutputStream;

class Connection {
    friend uint32_t runRead(void* lpParam);
    friend uint32_t runWrite(void* lpParam);
    friend uint32_t runSendAndQuit(void* lpParam);
    friend uint32_t runClose(void* lpParam);

private:
    static const int SEND_BUFFER_SIZE = 1024 * 5;

public:
    static int readThreads, writeThreads;

private:
    static const int MAX_TICKS_WITHOUT_INPUT = 20 * 60;

public:
    static const int IPTOS_LOWCOST = 0x02;
    static const int IPTOS_RELIABILITY = 0x04;
    static const int IPTOS_THROUGHPUT = 0x08;
    static const int IPTOS_LOWDELAY = 0x10;

private:
    Socket* socket;
    const SocketAddress* address;
    DataInputStream* dis;
    DataOutputStream*
        bufferedDos;  

    
    DataOutputStream* byteArrayDos;  
                                     
    ByteArrayOutputStream* baos;
    Socket::SocketOutputStream* sos;

    
    
    
    std::atomic<bool> running;

    std::queue<std::shared_ptr<Packet> >
        incoming;            
    std::mutex incoming_cs;  
    std::queue<std::shared_ptr<Packet> >
        outgoing;  
                   
    std::queue<std::shared_ptr<Packet> >
        outgoing_slow;  
                        

    PacketListener* packetListener;
    std::atomic<bool> quitting;

    C4JThread* readThread;
    C4JThread* writeThread;

    C4JThread::Event* m_hWakeReadThread;
    C4JThread::Event* m_hWakeWriteThread;

    uint32_t saqThreadID, closeThreadID;

    std::atomic<bool> disconnected;
    DisconnectPacket::eDisconnectReason disconnectReason;
    void** disconnectReasonObjects;  

    int noInputTicks;
    int estimatedRemaining;

    int tickCount;  

public:
    static int readSizes[256];
    static int writeSizes[256];

    int fakeLag;

private:
    void _init();

    
    std::mutex threadCounterLock;
    std::mutex writeLock;

public:
    ~Connection();
    Connection(Socket* socket, const std::wstring& id,
               PacketListener* packetListener);  

    void setListener(PacketListener* packetListener);
    void send(std::shared_ptr<Packet> packet);

public:
    void queueSend(std::shared_ptr<Packet> packet);

private:
    int slowWriteDelay;

    bool writeTick();

public:
    void flush();

private:
    bool readTick();

private:
    






public:
    void close(DisconnectPacket::eDisconnectReason reason);

    void tick();

    SocketAddress* getRemoteAddress();

    void sendAndQuit();

    int countDelayedPackets();

    Socket* getSocket() { return socket; }

private:
    static int runRead(void* lpParam);
    static int runWrite(void* lpParam);
    static int runClose(void* lpParam);
    static int runSendAndQuit(void* lpParam);
};
