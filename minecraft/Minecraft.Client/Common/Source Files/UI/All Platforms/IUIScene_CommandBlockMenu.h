#pragma once
#include <string>

#include "Minecraft.World/net/minecraft/world/level/tile/entity/net.minecraft.world.level.tile.entity.h"

class CommandBlockEntity;

class IUIScene_CommandBlockMenu {
public:
    virtual ~IUIScene_CommandBlockMenu() = default;
    void Initialise(CommandBlockEntity* commandBlock);

protected:
    void ConfirmButtonClicked();

    virtual std::wstring GetCommand() = 0;
    virtual void SetCommand(std::wstring command) = 0;
    virtual int GetPad() = 0;

private:
    CommandBlockEntity* m_commandBlock;
};
