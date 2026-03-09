#include "../../Minecraft.World/Platform/stdafx.h"
#include "UI.h"
#include "UIControl_Touch.h"

UIControl_Touch::UIControl_Touch()
{
}

bool UIControl_Touch::setupControl(UIScene *scene, IggyValuePath *parent, const std::string &controlName)
{
	UIControl::setControlType(UIControl::eTouchControl);
	bool success = UIControl_Base::setupControl(scene,parent,controlName);

	return success;
}

void UIControl_Touch::init(int iId)
{
	m_id = iId;

#if !defined(__linux__)
	switch(m_parentScene->GetParentLayer()->m_iLayer)
	{
	case eUILayer_Error:
	case eUILayer_Fullscreen:
	case eUILayer_Scene:
	case eUILayer_HUD:
		ui.TouchBoxAdd(this,m_parentScene);
		break;
	}
#endif
}

void UIControl_Touch::ReInit()
{
	UIControl_Base::ReInit();

	init(m_id);
}