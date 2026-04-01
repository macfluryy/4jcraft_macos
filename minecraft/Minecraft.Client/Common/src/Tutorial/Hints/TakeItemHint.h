#pragma once
// using namespace std;

#include "TutorialHint.h"
#include "Minecraft.Client/Common/src/Tutorial/TutorialEnum.h"

class ItemInstance;
class Tutorial;

class TakeItemHint : public TutorialHint {
private:
    int* m_iItems;
    unsigned int m_iItemsCount;

public:
    TakeItemHint(eTutorial_Hint id, Tutorial* tutorial, int items[],
                 unsigned int itemsLength);
    // TODO: 4jcraft, added, it was never implemented
    virtual ~TakeItemHint() {};

    virtual bool onTake(std::shared_ptr<ItemInstance> item);
};
