#pragma once

#include "UIControl_ButtonList.h"

class UIControl_SaveList : public UIControl_ButtonList
{
private:
	IggyName m_funcSetTextureName;

public:
	virtual bool setupControl(UIScene *scene, IggyValuePath *parent, const string &controlName);

	using UIControl_ButtonList::addItem;

	void addItem(const std::wstring &label);
	void addItem(const string &label);

	void addItem(const std::wstring &label, int data);
	void addItem(const string &label, int data);

	void addItem(const string &label, const std::wstring &iconName);
	void addItem(const std::wstring &label, const std::wstring &iconName);
	void setTextureName(int iId, const std::wstring &iconName);

private:
	void addItem(const string &label, const std::wstring &iconName, int data);
	void addItem(const std::wstring &label, const std::wstring &iconName, int data);

};