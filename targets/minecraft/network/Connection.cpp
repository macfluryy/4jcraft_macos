#include "Connection.h"

#include <stdio.h>

#include <chrono>
#include <thread>
#include <vector>

#include "platform/ShutdownManager.h"
#include "app/common/src/Network/GameNetworkManager.h"
#include "app/common/src/Network/NetworkPlayerInterface.h"
#include "app/common/src/Network/Socket.h"
#include "util/StringHelpers.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/compression.h"
#include "java/InputOutputStream/BufferedOutputStream.h"
#include "java/InputOutputStream/ByteArrayOutputStream.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "java/InputOutputStream/DataOutputStream.h"
#include "java/System.h"
#include "minecraft/network/packet/DisconnectPacket.h"
#include "minecraft/network/packet/KeepAlivePacket.h"
#include "minecraft/network/packet/Packet.h"
#include "minecraft/network/packet/PacketListener.h"

class SocketAddress;


#if !defined(_DEBUG)
#define CONNECTION_ENABLE_TIMEOUT_DISCONNECT 1
#endif

int Connection::readThreads = 0;
int Connection::writeThreads = 0;

int Connection::readSizes[256];
int Connection::writeSizes[256];

void Connection::_init() {
    
    running = true;
    quitting = false;
    disconnected = false;
    disconnectReason = DisconnectPacket::eDisconnect_None;
    disconnectReasonObjects = nullptr;
    noInputTicks = 0;
    estimatedRemaining = 0;
    fakeLag = 0;
    slowWriteDelay = 50;

    saqThreadID = 0;
    closeThreadID = 0;

    tickCount = 0;
}

Connection::~Connection() {
    
    
    running = false;
    if (dis)
        dis->close();  
                       
                       
    readThread->waitForCompletion(C4JThread::kInfiniteTimeout);
    writeThread->waitForCompletion(C4JThread::kInfiniteTimeout);

    delete m_hWakeReadThread;
    delete m_hWakeWriteThread;

    
    
    delete byteArrayDos;
    byteArrayDos = nullptr;
    delete baos;
    baos = nullptr;
    if (bufferedDos) {
        bufferedDos->deleteChildStream();
        delete bufferedDos;
        bufferedDos = nullptr;
    }
    delete dis;
    dis = nullptr;
}

Connection::Connection(Socket* socket, const std::wstring& id,
                       PacketListener* packetListener)  
{
    _init();

    this->socket = socket;

    address = socket->getRemoteSocketAddress();

    this->packetListener = packetListener;

    
    socket->setSoTimeout(30000);
    socket->setTrafficClass(IPTOS_THROUGHPUT | IPTOS_LOWDELAY);

    






    dis = new DataInputStream(
        socket->getInputStream(packetListener->isServerPacketListener()));

    sos = socket->getOutputStream(packetListener->isServerPacketListener());
    bufferedDos =
        new DataOutputStream(new BufferedOutputStream(sos, SEND_BUFFER_SIZE));
    baos = new ByteArrayOutputStream(SEND_BUFFER_SIZE);
    byteArrayDos = new DataOutputStream(baos);

    m_hWakeReadThread = new C4JThread::Event;
    m_hWakeWriteThread = new C4JThread::Event;

    std::string szId = wstringtofilename(id);
    char readThreadName[256];
    char writeThreadName[256];
    sprintf(readThreadName, "%s read\n", szId.c_str());
    sprintf(writeThreadName, "%s write\n", szId.c_str());

    readThread =
        new C4JThread(runRead, (void*)this, readThreadName, READ_STACK_SIZE);
    writeThread =
        new C4JThread(runWrite, this, writeThreadName, WRITE_STACK_SIZE);
    readThread->run();
    writeThread->run();

    












}

void Connection::setListener(PacketListener* packetListener) {
    this->packetListener = packetListener;
}

void Connection::send(std::shared_ptr<Packet> packet) {
    if (quitting) return;

    
    {
        std::lock_guard<std::mutex> lock(writeLock);

        estimatedRemaining += packet->getEstimatedSize() + 1;
        if (packet->shouldDelay) {
            
            
            packet->shouldDelay = false;
            outgoing_slow.push(packet);
        } else {
            outgoing.push(packet);
        }
    }

    
}

void Connection::queueSend(std::shared_ptr<Packet> packet) {
    if (quitting) return;
    {
        std::lock_guard<std::mutex> lock(writeLock);
        estimatedRemaining += packet->getEstimatedSize() + 1;
        outgoing_slow.push(packet);
    }
}

bool Connection::writeTick() {
    bool didSomething = false;

    
    
    if (bufferedDos == nullptr || byteArrayDos == nullptr) return didSomething;

    
    
    
    
    std::shared_ptr<Packet> packet;
    {
        std::lock_guard<std::mutex> lock(writeLock);
        if (!outgoing.empty() &&
            (fakeLag == 0 ||
             System::currentTimeMillis() - outgoing.front()->createTime >=
                 fakeLag)) {
            packet = outgoing.front();
            outgoing.pop();
            estimatedRemaining -= packet->getEstimatedSize() + 1;
        }
    }

    if (packet != nullptr) {
        Packet::writePacket(packet, bufferedDos);
#if defined(__linux__) && defined(__APPLE__)
        bufferedDos->flush();  
                               
#endif

#if !defined(_CONTENT_PACKAGE)
        
        int playerId = 0;
        if (!socket->isLocal()) {
            Socket* socket = getSocket();
            if (socket) {
                INetworkPlayer* player = socket->getPlayer();
                if (player) {
                    playerId = player->GetSmallId();
                }
            }
            Packet::recordOutgoingPacket(packet, playerId);
        }
#endif

        
        
        
        
        
        
        

        writeSizes[packet->getId()] += packet->getEstimatedSize() + 1;
        didSomething = true;
        packet.reset();
    }

    {
        std::lock_guard<std::mutex> lock(writeLock);
        if ((slowWriteDelay-- <= 0) && !outgoing_slow.empty() &&
            (fakeLag == 0 ||
             System::currentTimeMillis() -
                     outgoing_slow.front()->createTime >=
                 fakeLag)) {
            packet = outgoing_slow.front();
            outgoing_slow.pop();
            estimatedRemaining -= packet->getEstimatedSize() + 1;
        }
    }

    if (packet != nullptr) {
        
        
        
        
#if defined(__linux__) && defined(__APPLE__)
        
        
        
        
        Packet::writePacket(packet, bufferedDos);
        bufferedDos->flush();  
                               
#else
        if (packet->shouldDelay) {
            Packet::writePacket(packet, byteArrayDos);

            
            
            
            
            
            int flags = NON_QNET_SENDDATA_ACK_REQUIRED;
            sos->writeWithFlags(baos->buf, 0, baos->size(), flags);
            baos->reset();
        } else {
            Packet::writePacket(packet, bufferedDos);
        }

#endif

#if !defined(_CONTENT_PACKAGE)
        
        if (!socket->isLocal()) {
            int playerId = 0;
            if (!socket->isLocal()) {
                Socket* socket = getSocket();
                if (socket) {
                    INetworkPlayer* player = socket->getPlayer();
                    if (player) {
                        playerId = player->GetSmallId();
                    }
                }
                Packet::recordOutgoingPacket(packet, playerId);
            }
        }
#endif

        writeSizes[packet->getId()] += packet->getEstimatedSize() + 1;
        slowWriteDelay = 0;
        didSomething = true;
    }

    return didSomething;
}

void Connection::flush() {
    
    
    
    
    m_hWakeReadThread->set();
    m_hWakeWriteThread->set();
}

bool Connection::readTick() {
    bool didSomething = false;

    
    
    if (dis == nullptr) return didSomething;

    

    std::shared_ptr<Packet> packet =
        Packet::readPacket(dis, packetListener->isServerPacketListener());

    if (packet != nullptr) {
        readSizes[packet->getId()] += packet->getEstimatedSize() + 1;
        
        
        
        bool wasQueued = false;
        {
            std::lock_guard<std::mutex> lock(incoming_cs);
            if (!quitting) {
                incoming.push(packet);
                wasQueued = true;
            }
        }
        


 didSomething = true;
    } else {
        

        
        
        
        
        
    }

    





    return didSomething;
}








void Connection::close(DisconnectPacket::eDisconnectReason reason) {
    
    if (!running) return;
    
    disconnected = true;

    disconnectReason = reason;  
    disconnectReasonObjects = nullptr;

    
    
    
    
    
    
    
    
    
    
    
    

    

    running = false;

    if (dis)
        dis->close();  
                       
                       

    
    
    readThread->waitForCompletion(C4JThread::kInfiniteTimeout);
    writeThread->waitForCompletion(C4JThread::kInfiniteTimeout);

    delete dis;
    dis = nullptr;
    if (bufferedDos) {
        bufferedDos->close();
        bufferedDos->deleteChildStream();
        delete bufferedDos;
        bufferedDos = nullptr;
    }
    if (byteArrayDos) {
        byteArrayDos->close();
        delete byteArrayDos;
        byteArrayDos = nullptr;
    }
    if (socket) {
        socket->close(packetListener->isServerPacketListener());
        socket = nullptr;
    }

    
    
    
    
    
    
    
    {
        std::lock_guard<std::mutex> lock(writeLock);
        std::queue<std::shared_ptr<Packet> > emptyA;
        std::queue<std::shared_ptr<Packet> > emptyB;
        outgoing.swap(emptyA);
        outgoing_slow.swap(emptyB);
        estimatedRemaining = 0;
    }
}

void Connection::tick() {
    if (estimatedRemaining > 1 * 1024 * 1024) {
        close(DisconnectPacket::eDisconnect_Overflow);
    }
    bool empty;
    {
        std::lock_guard<std::mutex> lock(incoming_cs);
        empty = incoming.empty();
    }
    if (empty) {
#if CONNECTION_ENABLE_TIMEOUT_DISCONNECT
        
        
        
        
        
        if (noInputTicks++ >= MAX_TICKS_WITHOUT_INPUT) {
            close(DisconnectPacket::eDisconnect_TimeOut);
        }
#endif
    }
    
    
    
    
    
    
    
    else {
        noInputTicks = 0;
    }

    
    
    tickCount++;
    if (tickCount % 20 == 0) {
        send(std::make_shared<KeepAlivePacket>());
    }

    
    int max = 1000;

    
    
    
    
    
    
    
    
    

    
    std::vector<std::shared_ptr<Packet> > packetsToHandle;
    {
        std::lock_guard<std::mutex> lock(incoming_cs);
        while (!disconnected && !g_NetworkManager.IsLeavingGame() &&
               g_NetworkManager.IsInSession() && !incoming.empty() &&
               max-- >= 0) {
            std::shared_ptr<Packet> packet = incoming.front();
            packetsToHandle.push_back(packet);
            incoming.pop();
        }
    }

    
    
    
    
    if (packetsToHandle.size() > 1) {
        

    }
    
    
    for (int i = 0; i < packetsToHandle.size(); i++) {
        
        
        
        




        packetsToHandle[i]->handle(packetListener);
    }
    if (packetsToHandle.size() > 1) {
        

    }
    flush();

    
    
    
    if (socket && socket->isClosing()) {
        close(DisconnectPacket::eDisconnect_Closed);
    }

    
    
    if (disconnected) {
        bool empty;
        {
            std::lock_guard<std::mutex> lock(incoming_cs);
            empty = incoming.empty();
        }
        if (empty) {
            packetListener->onDisconnect(disconnectReason,
                                         disconnectReasonObjects);
            disconnected =
                false;  
        }
    }
}

SocketAddress* Connection::getRemoteAddress() {
    return (SocketAddress*)address;
}

void Connection::sendAndQuit() {
    if (quitting) {
        return;
    }
    
    flush();
    quitting = true;
    
    
    

    
    
    
    if (running) {
        
        close(DisconnectPacket::eDisconnect_Closed);
    }
}

int Connection::countDelayedPackets() {
    std::lock_guard<std::mutex> lock(writeLock);
    return (int)outgoing_slow.size();
}

int Connection::runRead(void* lpParam) {
    ShutdownManager::HasStarted(ShutdownManager::eConnectionReadThreads);
    Connection* con = (Connection*)lpParam;

    if (con == nullptr) {
        return 0;
    }

    Compression::UseDefaultThreadStorage();

    std::mutex* cs = &con->threadCounterLock;

    {
        std::lock_guard<std::mutex> lock(*cs);
        con->readThreads++;
    }

    

    while (
        con->running && !con->quitting &&
        ShutdownManager::ShouldRun(ShutdownManager::eConnectionReadThreads)) {
        while (con->readTick());

        
        
        
        
        con->m_hWakeReadThread->waitForSignal(100L);
    }

    









    ShutdownManager::HasFinished(ShutdownManager::eConnectionReadThreads);
    return 0;
}

int Connection::runWrite(void* lpParam) {
    ShutdownManager::HasStarted(ShutdownManager::eConnectionWriteThreads);
    Connection* con = dynamic_cast<Connection*>((Connection*)lpParam);

    if (con == nullptr) {
        ShutdownManager::HasFinished(ShutdownManager::eConnectionWriteThreads);
        return 0;
    }

    Compression::UseDefaultThreadStorage();

    std::mutex* cs = &con->threadCounterLock;

    {
        std::lock_guard<std::mutex> lock(*cs);
        con->writeThreads++;
    }

    
    
    
    
    unsigned int waitResult = C4JThread::WaitResult::Timeout;

    while (
        (con->running || waitResult == C4JThread::WaitResult::Signaled) &&
        ShutdownManager::ShouldRun(ShutdownManager::eConnectionWriteThreads)) {
        while (con->writeTick());

        
        
        
        waitResult = con->m_hWakeWriteThread->waitForSignal(100L);

        if (con->bufferedDos != nullptr) con->bufferedDos->flush();
        
    }

    
    {
        std::lock_guard<std::mutex> lock(*cs);
        con->writeThreads--;
    }

    ShutdownManager::HasFinished(ShutdownManager::eConnectionWriteThreads);
    return 0;
}

int Connection::runClose(void* lpParam) {
    Connection* con = dynamic_cast<Connection*>((Connection*)lpParam);

    if (con == nullptr) return 0;

    

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    if (con->running) {
        
        con->close(DisconnectPacket::eDisconnect_Closed);
    }

    




    return 1;
}

int Connection::runSendAndQuit(void* lpParam) {
    Connection* con = dynamic_cast<Connection*>((Connection*)lpParam);
    

    if (con == nullptr) return 0;

    

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    if (con->running) {
        
        con->close(DisconnectPacket::eDisconnect_Closed);
        
    }

    
    




    return 0;
}
