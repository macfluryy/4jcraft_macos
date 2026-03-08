#include "../../Minecraft.World/Build/stdafx.h"

#include "../../Minecraft.World/Containers/Slot.h"
#include "../../Minecraft.World/Items/ItemInstance.h"

#include "SlotItemControlBase.h"
#include "SlotProgressControl.h"

int SlotProgressControl::GetValue()
{
	int value = 0;

	HXUIOBJ hVisual, hParent;
	this->GetParent( &hVisual );
	XuiElementGetParent( hVisual, &hParent);

	void* pvUserData;
	XuiElementGetUserData( hParent, &pvUserData );

	if( pvUserData != NULL )
	{
		SlotControlUserDataContainer* pUserDataContainer = (SlotControlUserDataContainer*)pvUserData;
	
		std::shared_ptr<ItemInstance> item = std::shared_ptr<ItemInstance>();

		if( pUserDataContainer->slot != NULL )
		{
			item = pUserDataContainer->slot->getItem();
		}
		else
		{
			item = pUserDataContainer->item;
		}

		if( item != NULL )
		{
			// TODO Should use getDamage instead even though it returns the same value
			if( item->isDamaged() )
				value = item->getDamageValue();
			else
				value = 0;
		}
	}
	else
	{
		LPCWSTR name;
		XuiElementGetId( hParent, &name );

		OutputDebugStringW( name );
		OutputDebugString( "\n" );
	}

	return value;
}

void SlotProgressControl::GetRange(int *pnRangeMin, int *pnRangeMax)
{
	*pnRangeMin = 0;
	*pnRangeMax = 0;

	HXUIOBJ hVisual, hParent;
	this->GetParent( &hVisual );
	XuiElementGetParent( hVisual, &hParent);

	void* pvUserData;
	XuiElementGetUserData( hParent, &pvUserData );

	if( pvUserData != NULL )
	{
		SlotControlUserDataContainer* pUserDataContainer = (SlotControlUserDataContainer*)pvUserData;
	
		std::shared_ptr<ItemInstance> item = std::shared_ptr<ItemInstance>();

		if( pUserDataContainer->slot != NULL )
		{
			item = pUserDataContainer->slot->getItem();
		}
		else
		{
			item = pUserDataContainer->item;
		}

		if( item != NULL )
		{
			*pnRangeMax = item->getMaxDamage();
		}
	}
}