#include "../../../../Minecraft.World/Build/stdafx.h"
#include "../../../../Minecraft.World/Items/ItemInstance.h"
#include "CompleteUsingItemTask.h"

CompleteUsingItemTask::CompleteUsingItemTask(Tutorial *tutorial, int descriptionId, int itemIds[], unsigned int itemIdsLength, bool enablePreCompletion)
	: TutorialTask( tutorial, descriptionId, enablePreCompletion, NULL)
{	
	m_iValidItemsA= new int [itemIdsLength];
	for(int i=0;i<itemIdsLength;i++)
	{
		m_iValidItemsA[i]=itemIds[i];
	}
	m_iValidItemsCount = itemIdsLength;
}

CompleteUsingItemTask::~CompleteUsingItemTask()
{
	delete [] m_iValidItemsA;
}

bool CompleteUsingItemTask::isCompleted()
{
	return bIsCompleted;
}

void CompleteUsingItemTask::completeUsingItem(std::shared_ptr<ItemInstance> item)
{
	if(!hasBeenActivated() && !isPreCompletionEnabled()) return;
	for(int i=0;i<m_iValidItemsCount;i++)
	{
		if( item->id == m_iValidItemsA[i] )
		{
			bIsCompleted = true;
			break;
		}
	}
}