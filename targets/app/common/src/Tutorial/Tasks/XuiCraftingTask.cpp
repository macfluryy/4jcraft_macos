#include "XuiCraftingTask.h"

#include "app/common/src/Tutorial/Tutorial.h"
#include "app/common/src/UI/Scenes/In-Game Menu Screens/UIScene_CraftingMenu.h"

bool XuiCraftingTask::isCompleted() {
    
    
    
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
