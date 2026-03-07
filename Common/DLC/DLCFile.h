#pragma once
#include "DLCManager.h"

class DLCFile
{
protected:
	DLCManager::EDLCType m_type;
	std::wstring m_path;
	DWORD m_dwSkinId;

public:
	DLCFile(DLCManager::EDLCType type, const std::wstring &path);
	virtual ~DLCFile() {}

	DLCManager::EDLCType getType()	{ return m_type; }
	std::wstring getPath()				{ return m_path; }
	DWORD getSkinID()				{ return m_dwSkinId; }

	virtual void addData(PBYTE pbData, DWORD dwBytes) {}
	virtual PBYTE getData(DWORD &dwBytes) { dwBytes = 0; return NULL; }
	virtual void addParameter(DLCManager::EDLCParameterType type, const std::wstring &value) {}

	virtual std::wstring getParameterAsString(DLCManager::EDLCParameterType type) { return L""; }
	virtual bool getParameterAsBool(DLCManager::EDLCParameterType type) { return false;}
};