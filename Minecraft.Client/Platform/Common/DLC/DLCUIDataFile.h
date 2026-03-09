#pragma once
#include "DLCFile.h"

class DLCUIDataFile : public DLCFile
{
private:
	uint8_t *m_pbData;
	DWORD m_dwBytes;
	bool m_canDeleteData;

public:
	DLCUIDataFile(const std::wstring &path);
	~DLCUIDataFile();

	using DLCFile::addData;
	using DLCFile::addParameter;

	virtual void addData(uint8_t *pbData, DWORD dwBytes,bool canDeleteData = false);
	virtual uint8_t *getData(DWORD &dwBytes);
};
