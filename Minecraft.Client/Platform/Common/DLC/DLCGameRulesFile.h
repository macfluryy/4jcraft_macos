#pragma once
#include "DLCGameRules.h"

class DLCGameRulesFile : public DLCGameRules
{
private:
	uint8_t *m_pbData;
	DWORD m_dwBytes;

public:
	DLCGameRulesFile(const std::wstring &path);

	virtual void addData(uint8_t *pbData, DWORD dwBytes);
	virtual uint8_t *getData(DWORD &dwBytes);
};
