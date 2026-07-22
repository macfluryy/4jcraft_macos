#pragma once

#include <stdint.h>

#include <string>

#include "platform/PlatformTypes.h"
#include "NetworkPlayerInterface.h"

class IQNetPlayer;
class Socket;






class NetworkPlayerQNet : public INetworkPlayer {
public:
    
    NetworkPlayerQNet(IQNetPlayer* qnetPlayer);
    virtual unsigned char GetSmallId();
    virtual void SendData(INetworkPlayer* player, const void* pvData,
                          int dataSize, bool lowPriority, bool ack);
    virtual bool IsSameSystem(INetworkPlayer* player);
    virtual int GetOutstandingAckCount();
    virtual int GetSendQueueSizeBytes(INetworkPlayer* player, bool lowPriority);
    virtual int GetSendQueueSizeMessages(INetworkPlayer* player,
                                         bool lowPriority);
    virtual int GetCurrentRtt();
    virtual bool IsHost();
    virtual bool IsGuest();
    virtual bool IsLocal();
    virtual int GetSessionIndex();
    virtual bool IsTalking();
    virtual bool IsMutedByLocalUser(int userIndex);
    virtual bool HasVoice();
    virtual bool HasCamera();
    virtual int GetUserIndex();
    virtual void SetSocket(Socket* pSocket);
    virtual Socket* GetSocket();
    virtual const wchar_t* GetOnlineName();
    virtual std::wstring GetDisplayName();
    virtual PlayerUID GetUID();
    virtual void SentChunkPacket();
    virtual int GetTimeSinceLastChunkPacket_ms();

    IQNetPlayer* GetQNetPlayer();

private:
    IQNetPlayer* m_qnetPlayer;
    Socket* m_pSocket;
    int64_t m_lastChunkPacketTime;
};