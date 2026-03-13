#include "../Platform/stdafx.h"
#include "Language.h"

// 4J - TODO - properly implement

Language *Language::singleton = nullptr;

Language::Language()
{
}

Language *Language::getInstance()
{
	// 4jcraft, fixes static init fiassco in I18n.cpp
	if(singleton == nullptr) {
		singleton = new Language();
	}

	return singleton;
}

/* 4J Jev, creates 2 identical functions.
std::wstring Language::getElement(const std::wstring& elementId)
{
	return elementId;
} */

// 4jcraft changed, again const reference into va_start, std forbids
std::wstring Language::getElement(std::wstring elementId, ...)
{
#ifdef __PSVITA__		// 4J - vita doesn't like having a reference type as the last parameter passed to va_start - we shouldn't need this method anyway
	return L"";
#else
	va_list args;
	va_start(args, elementId);
	return getElement(elementId, args);
#endif
}

std::wstring Language::getElement(const std::wstring& elementId, va_list args)
{
	// 4J TODO
	return elementId;
}

std::wstring Language::getElementName(const std::wstring& elementId)
{
	return elementId;
}

std::wstring Language::getElementDescription(const std::wstring& elementId)
{
	return elementId;
}
