#pragma once

#include <string>

#include "TileEntity.h"
#include "Minecraft.World/net/minecraft/commands/CommandSender.h"
#include "java/Class.h"
#include "Minecraft.World/net/minecraft/commands/CommandsEnum.h"
#include "Minecraft.World/net/minecraft/network/packet/ChatPacket.h"

class ChatMessageComponent;
class Level;
class Pos;

class CommandBlockEntity : public TileEntity, public CommandSender {
public:
    eINSTANCEOF GetType() { return eTYPE_COMMANDBLOCKTILEENTITY; }
    static TileEntity* create() { return new CommandBlockEntity(); }

    // 4J Added
    virtual std::shared_ptr<TileEntity> clone();

private:
    int successCount;
    std::wstring command;
    std::wstring name;

public:
    CommandBlockEntity();

    void setCommand(const std::wstring& command);
    std::wstring getCommand();
    int performCommand(Level* level);
    std::wstring getName();
    void setName(const std::wstring& name);
    virtual void sendMessage(
        const std::wstring& message,
        ChatPacket::EChatPacketMessage type = ChatPacket::e_ChatCustom,
        int customData = -1, const std::wstring& additionalMessage = L"");
    virtual bool hasPermission(EGameCommand command);
    // void sendMessage(ChatMessageComponent *message);
    // bool hasPermission(int permissionLevel, const std::wstring &commandName);
    void save(CompoundTag* tag);
    void load(CompoundTag* tag);
    Pos* getCommandSenderWorldPosition();
    Level* getCommandSenderWorld();
    std::shared_ptr<Packet> getUpdatePacket();
    int getSuccessCount();
    void setSuccessCount(int successCount);
};