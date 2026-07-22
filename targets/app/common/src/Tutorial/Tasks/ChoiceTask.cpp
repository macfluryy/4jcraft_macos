#include "ChoiceTask.h"

#include <memory>
#include <vector>

#include "platform/sdl2/Input.h"
#include "app/common/src/Tutorial/Constraints/InputConstraint.h"
#include "app/common/src/Tutorial/Tasks/TutorialTask.h"
#include "app/common/src/Tutorial/Tutorial.h"
#include "app/common/src/Tutorial/TutorialEnum.h"
#include "app/mac/Mac_UIController.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/multiplayer/MultiPlayerLocalPlayer.h"
#include "minecraft/world/level/material/Material.h"

ChoiceTask::ChoiceTask(
    Tutorial* tutorial, int descriptionId, int promptId ,
    bool requiresUserInput , int iConfirmMapping ,
    int iCancelMapping ,
    eTutorial_CompletionAction cancelAction )
    : TutorialTask(tutorial, descriptionId, false, nullptr, true, false,
                   false) {
    if (requiresUserInput == true) {
        constraints.push_back(new InputConstraint(iConfirmMapping));
        constraints.push_back(new InputConstraint(iCancelMapping));
    }
    m_iConfirmMapping = iConfirmMapping;
    m_iCancelMapping = iCancelMapping;
    m_bConfirmMappingComplete = false;
    m_bCancelMappingComplete = false;

    m_cancelAction = cancelAction;

    m_promptId = promptId;
    tutorial->addMessage(m_promptId);
}

bool ChoiceTask::isCompleted() {
    Minecraft* pMinecraft = Minecraft::GetInstance();

    if (m_bConfirmMappingComplete || m_bCancelMappingComplete) {
        enableConstraints(false, true);
        return true;
    }

    if (ui.GetMenuDisplayed(tutorial->getPad())) {
        
        
    } else {
        
        
        if (pMinecraft->localplayers[tutorial->getPad()]->isUnderLiquid(
                Material::water))
            return false;

        if (!m_bConfirmMappingComplete &&
            InputManager.GetValue(pMinecraft->player->GetXboxPad(),
                                  m_iConfirmMapping) > 0) {
            m_bConfirmMappingComplete = true;
        }
        if (!m_bCancelMappingComplete &&
            InputManager.GetValue(pMinecraft->player->GetXboxPad(),
                                  m_iCancelMapping) > 0) {
            m_bCancelMappingComplete = true;
        }
    }

    if (m_bConfirmMappingComplete || m_bCancelMappingComplete) {
        enableConstraints(false, true);
    }
    return m_bConfirmMappingComplete || m_bCancelMappingComplete;
}

eTutorial_CompletionAction ChoiceTask::getCompletionAction() {
    if (m_bCancelMappingComplete) {
        return m_cancelAction;
    } else {
        return e_Tutorial_Completion_None;
    }
}

int ChoiceTask::getPromptId() {
    if (m_bShownForMinimumTime)
        return m_promptId;
    else
        return -1;
}

void ChoiceTask::setAsCurrentTask(bool active ) {
    enableConstraints(active);
    TutorialTask::setAsCurrentTask(active);
}

void ChoiceTask::handleUIInput(int iAction) {
    if (bHasBeenActivated && m_bShownForMinimumTime) {
        if (iAction == m_iConfirmMapping) {
            m_bConfirmMappingComplete = true;
        } else if (iAction == m_iCancelMapping) {
            m_bCancelMappingComplete = true;
        }
    }
}
