#include "../../Minecraft.World/Platform/stdafx.h"
#include "../../Minecraft.World/Items/ItemInstance.h"
#if !(0) && !(0)
#include "../UI/UI.h"
#endif
#include "Tutorial.h"
#include "XuiCraftingTask.h"

bool XuiCraftingTask::isCompleted() {
#if 1
    // This doesn't seem to work
    // IUIScene_CraftingMenu *craftScene =
    // reinterpret_cast<IUIScene_CraftingMenu *>(tutorial->getScene());
#if 0
    CXuiSceneCraftingPanel* craftScene =
        (CXuiSceneCraftingPanel*)(tutorial->getScene());
#else
    UIScene_CraftingMenu* craftScene =
        reinterpret_cast<UIScene_CraftingMenu*>(tutorial->getScene());
#endif

    bool completed = false;

    switch (m_type) {
        case e_Crafting_SelectGroup:
            if (craftScene != NULL &&
                craftScene->getCurrentGroup() == m_group) {
                completed = true;
            }
            break;
        case e_Crafting_SelectItem:
            if (craftScene != NULL && craftScene->isItemSelected(m_item)) {
                completed = true;
            }
            break;
    }

    return completed;
#else
    return true;
#endif
}
