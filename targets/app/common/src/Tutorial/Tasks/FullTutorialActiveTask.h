#pragma once


#include "app/common/src/Tutorial/TutorialEnum.h"
#include "TutorialTask.h"

class Tutorial;


class FullTutorialActiveTask : public TutorialTask {
private:
    eTutorial_CompletionAction m_completeAction;

    bool CompletionMaskIsValid();

public:
    FullTutorialActiveTask(
        Tutorial* tutorial,
        eTutorial_CompletionAction completeAction = e_Tutorial_Completion_None);
    virtual bool isCompleted();
    virtual eTutorial_CompletionAction getCompletionAction();
};