#pragma once
//using namespace std;

#include "../../Minecraft.World/Util/Class.h"
#include "TutorialHint.h"

class ItemInstance;

class LookAtEntityHint : public TutorialHint
{
private:
	eINSTANCEOF m_type;
	int m_titleId;

public:
	LookAtEntityHint(eTutorial_Hint id, Tutorial *tutorial, int descriptionId, int titleId, eINSTANCEOF type);
	//TODO: LEAK LEKAFDSAKFDJ;SALKDJF;ASLKFJ ITS NEVER IMPLEMENTED
	~LookAtEntityHint(){};

	virtual bool onLookAtEntity(eINSTANCEOF type);
};
