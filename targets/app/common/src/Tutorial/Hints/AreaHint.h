#pragma once

#include "app/common/src/Tutorial/TutorialEnum.h"
#include "TutorialHint.h"
#include "minecraft/world/phys/AABB.h"

class AABB;
class Tutorial;

class AreaHint : public TutorialHint {
private:
    AABB area;
    bool contains;  
                    

    
    eTutorial_State m_displayState;

    
    eTutorial_State m_completeState;

public:
    AreaHint(eTutorial_Hint id, Tutorial* tutorial,
             eTutorial_State displayState, eTutorial_State completeState,
             int descriptionId, double x0, double y0, double z0, double x1,
             double y1, double z1, bool allowFade = true, bool contains = true);

    virtual int tick();
};
