#pragma once


#include "TutorialTask.h"

class Stat;
class Tutorial;



class StatTask : public TutorialTask {
private:
    Stat* stat;
    int targetValue;

public:
    StatTask(Tutorial* tutorial, int descriptionId, bool enablePreCompletion,
             Stat* stat, int variance = 1);
    virtual bool isCompleted();
};