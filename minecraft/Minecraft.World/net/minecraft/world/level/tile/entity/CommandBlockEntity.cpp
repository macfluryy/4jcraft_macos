#include <assert.h>
#include <memory>

#include "CommandBlockEntity.h"
#include "nbt/CompoundTag.h"
#include "Minecraft.World/net/minecraft/Pos.h"
#include "Minecraft.World/net/minecraft/network/packet/TileEntityDataPacket.h"
#include "Minecraft.World/net/minecraft/world/level/tile/entity/TileEntity.h"

class Level;

CommandBlockEntity::CommandBlockEntity() {
    successCount = 0;
    command = L"";
    name = L"@";
}

void CommandBlockEntity::setCommand(const std::wstring& command) {
    this->command = command;
    setChanged();
}

std::wstring CommandBlockEntity::getCommand() { return command; }

int CommandBlockEntity::performCommand(Level* level) {
    // 4J-JEV: Cannot decide what to do with the command field.
    assert(false);
    return 0;
}

std::wstring CommandBlockEntity::getName() { return name; }

void CommandBlockEntity::setName(const std::wstring& name) {
    this->name = name;
}

void CommandBlockEntity::sendMessage(const std::wstring& message,
                                     ChatPacket::EChatPacketMessage type,
                                     int customData,
                                     const std::wstring& additionalMessage) {}

bool CommandBlockEntity::hasPermission(EGameCommand command) { return false; }

void CommandBlockEntity::save(CompoundTag* tag) {
    TileEntity::save(tag);
    tag->putString(L"Command", command);
    tag->putInt(L"SuccessCount", successCount);
    tag->putString(L"CustomName", name);
}

void CommandBlockEntity::load(CompoundTag* tag) {
    TileEntity::load(tag);
    command = tag->getString(L"Command");
    successCount = tag->getInt(L"SuccessCount");
    if (tag->contains(L"CustomName")) name = tag->getString(L"CustomName");
}

Pos* CommandBlockEntity::getCommandSenderWorldPosition() {
    return new Pos(x, y, z);
}

Level* CommandBlockEntity::getCommandSenderWorld() { return getLevel(); }

std::shared_ptr<Packet> CommandBlockEntity::getUpdatePacket() {
    CompoundTag* tag = new CompoundTag();
    save(tag);
    return std::shared_ptr<TileEntityDataPacket>(new TileEntityDataPacket(
        x, y, z, TileEntityDataPacket::TYPE_ADV_COMMAND, tag));
}

int CommandBlockEntity::getSuccessCount() { return successCount; }

void CommandBlockEntity::setSuccessCount(int successCount) {
    this->successCount = successCount;
}

// 4J Added
std::shared_ptr<TileEntity> CommandBlockEntity::clone() {
    std::shared_ptr<CommandBlockEntity> result =
        std::shared_ptr<CommandBlockEntity>(new CommandBlockEntity());
    TileEntity::clone(result);

    result->successCount = successCount;
    result->command = command;
    result->name = name;

    return result;
}