#include "../../Minecraft.World/Platform/stdafx.h"
#include "../../Minecraft.World/Items/ItemInstance.h"
#include "../UI/UI.h"
#include "Tutorial.h"
#include "XuiCraftingTask.h"

bool XuiCraftingTask::isCompleted() {
    // This doesn't seem to work
    // IUIScene_CraftingMenu *craftScene =
    // reinterpret_cast<IUIScene_CraftingMenu *>(tutorial->getScene());
    UIScene_CraftingMenu* craftScene =
        reinterpret_cast<UIScene_CraftingMenu*>(tutorial->getScene());

    bool completed = false;

    switch (m_type) {
        case e_Crafting_SelectGroup:
            if (craftScene != nullptr &&
                craftScene->getCurrentGroup() == m_group) {
                completed = true;
            }
            break;
        case e_Crafting_SelectItem:
            if (craftScene != nullptr && craftScene->isItemSelected(m_item)) {
                completed = true;
            }
            break;
    }

    return completed;
}
