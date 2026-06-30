#pragma once

#include <memory>
#include <string>

#include "Packet.h"

class DisconnectPacket : public Packet,
                         public std::enable_shared_from_this<DisconnectPacket> {
public:
    enum eDisconnectReason {
        eDisconnect_None = 0,
        eDisconnect_Quitting,
        eDisconnect_Closed,

        eDisconnect_LoginTooLong,
        eDisconnect_IllegalStance,
        eDisconnect_IllegalPosition,
        eDisconnect_MovedTooQuickly,
        eDisconnect_NoFlying,
        eDisconnect_Kicked,

        eDisconnect_TimeOut,
        eDisconnect_Overflow,
        eDisconnect_EndOfStream,
        eDisconnect_ServerFull,
        eDisconnect_OutdatedServer,
        eDisconnect_OutdatedClient,
        eDisconnect_UnexpectedPacket,

        eDisconnect_ConnectionCreationFailed,
        eDisconnect_NoMultiplayerPrivilegesHost,
        eDisconnect_NoMultiplayerPrivilegesJoin,

        eDisconnect_NoUGC_AllLocal,
        eDisconnect_NoUGC_Single_Local,
        eDisconnect_ContentRestricted_AllLocal,
        eDisconnect_ContentRestricted_Single_Local,
        eDisconnect_NoUGC_Remote,

        eDisconnect_NoFriendsInGame,
        eDisconnect_Banned,
        eDisconnect_NotFriendsWithHost,
        eDisconnect_NATMismatch,

        // 4J macOS - sentinel: the wire carries an arbitrary disconnect string
        // after the reason int. Used by the Java-edition proxy to forward a
        // server's flattened Java kick message verbatim. Added at the END so
        // every existing code value is unchanged (vanilla servers never send
        // it, so their packets read exactly as before).
        eDisconnect_CustomText,
    };

    // 4J Stu - The reason was a string, but we need to send a non-locale
    // specific reason
    eDisconnectReason reason;

    // 4J macOS - only populated when reason == eDisconnect_CustomText; the raw,
    // already-flattened disconnect message to show on the disconnect screen.
    std::wstring m_customText;

    DisconnectPacket();
    DisconnectPacket(eDisconnectReason reason);
    explicit DisconnectPacket(const std::wstring& customText);

    virtual void read(DataInputStream* dis);
    virtual void write(DataOutputStream* dos);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    virtual bool canBeInvalidated();
    virtual bool isInvalidatedBy(std::shared_ptr<Packet> packet);

public:
    static std::shared_ptr<Packet> create() {
        return std::make_shared<DisconnectPacket>();
    }
    virtual int getId() { return 255; }
};
