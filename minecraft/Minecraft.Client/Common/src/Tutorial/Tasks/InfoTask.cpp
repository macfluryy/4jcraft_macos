#include <unordered_map>
#include <memory>
#include <utility>
#include <vector>

#include "Minecraft.Client/net/minecraft/client/Minecraft.h"
#include "Minecraft.Client/net/minecraft/client/multiplayer/MultiPlayerLocalPlayer.h"
#include "Minecraft.Client/Common/src/Tutorial/Tutorial.h"
#include "InfoTask.h"
#include "Minecraft.World/net/minecraft/world/level/material/Material.h"
#include "4J_Input.h"
#include "Minecraft.Client/Common/src/Tutorial/Constraints/InputConstraint.h"
#include "Minecraft.Client/Common/src/Tutorial/Tasks/TutorialTask.h"
#include "Minecraft.Client/Linux/Linux_UIController.h"

InfoTask::InfoTask(
    Tutorial* tutorial, int descriptionId, int promptId /*= -1*/,
    bool requiresUserInput /*= false*/, int iMapping /*= 0*/)
    : TutorialTask(tutorial, descriptionId, false, nullptr, true, false,
                   false) {
    if (requiresUserInput == true) {
        constraints.push_back(new InputConstraint(iMapping));
    }
    completedMappings[iMapping] = false;

    m_promptId = promptId;
    tutorial->addMessage(m_promptId);
}

bool InfoTask::isCompleted() {
    if (bIsCompleted) return true;

    if (tutorial->m_hintDisplayed) return false;

    if (!bHasBeenActivated || !m_bShownForMinimumTime) return false;

    bool bAllComplete = true;

    Minecraft* pMinecraft = Minecraft::GetInstance();

    // If the player is under water then allow all keypresses so they can jump
    // out
    if (pMinecraft->localplayers[tutorial->getPad()]->isUnderLiquid(
            Material::water))
        return false;

    if (ui.GetMenuDisplayed(tutorial->getPad())) {
        // If a menu is displayed, then we use the handleUIInput to complete the
        // task
        bAllComplete = true;
        for (auto it = completedMappings.begin(); it != completedMappings.end();
             ++it) {
            bool current = (*it).second;
            if (!current) {
                bAllComplete = false;
                break;
            }
        }
    } else {
        int iCurrent = 0;

        for (auto it = completedMappings.begin(); it != completedMappings.end();
             ++it) {
            bool current = (*it).second;
            if (!current) {
                if (InputManager.GetValue(pMinecraft->player->GetXboxPad(),
                                          (*it).first) > 0) {
                    (*it).second = true;
                    bAllComplete = true;
                } else {
                    bAllComplete = false;
                }
            }
            iCurrent++;
        }
    }

    if (bAllComplete == true) {
        enableConstraints(false, true);
    }
    bIsCompleted = bAllComplete;
    return bAllComplete;
}

int InfoTask::getPromptId() {
    if (m_bShownForMinimumTime)
        return m_promptId;
    else
        return -1;
}

void InfoTask::setAsCurrentTask(bool active /*= true*/) {
    enableConstraints(active);
    TutorialTask::setAsCurrentTask(active);
}

void InfoTask::handleUIInput(int iAction) {
    if (bHasBeenActivated) {
        for (auto it = completedMappings.begin(); it != completedMappings.end();
             ++it) {
            if (iAction == (*it).first) {
                (*it).second = true;
            }
        }
    }
}
