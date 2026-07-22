#pragma once


#include <format>
#include <vector>

#include "TutorialTask.h"

class Level;
class Tutorial;
class TutorialConstraint;


class UseItemTask : public TutorialTask {
private:
    const int itemId;

public:
    UseItemTask(const int itemId, Tutorial* tutorial, int descriptionId,
                bool enablePreCompletion = false,
                std::vector<TutorialConstraint*>* inConstraints = nullptr,
                bool bShowMinimumTime = false, bool bAllowFade = true,
                bool bTaskReminders = true);
    virtual bool isCompleted();
    virtual void useItem(std::shared_ptr<ItemInstance> item,
                         bool bTestUseOnly = false);
};