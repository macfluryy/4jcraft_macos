#pragma once
#include "../../../../Minecraft.World/Headers/net.minecraft.world.level.tile.entity.h"

class IUIScene_CommandBlockMenu {
public:
    void Initialise(CommandBlockEntity* commandBlock);

protected:
    void ConfirmButtonClicked();

    virtual std::wstring GetCommand();
    virtual void SetCommand(std::wstring command);
    virtual int GetPad();

private:
    CommandBlockEntity* m_commandBlock;
};