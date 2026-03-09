#pragma once
#include "DLCFile.h"

class DLCTextureFile : public DLCFile
{

private:
	bool m_bIsAnim;
	std::wstring m_animString;

	uint8_t *m_pbData;
	DWORD m_dwBytes;

public:
	DLCTextureFile(const std::wstring &path);

	virtual void addData(uint8_t *pbData, DWORD dwBytes);
	virtual uint8_t *getData(DWORD &dwBytes);

	virtual void addParameter(DLCManager::EDLCParameterType type, const std::wstring &value);

	virtual std::wstring getParameterAsString(DLCManager::EDLCParameterType type);
	virtual bool getParameterAsBool(DLCManager::EDLCParameterType type);
};
