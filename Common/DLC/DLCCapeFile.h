#pragma once
#include "DLCFile.h"

class DLCCapeFile : public DLCFile
{
public:
	DLCCapeFile(const std::wstring &path);

	virtual void addData(PBYTE pbData, DWORD dwBytes);
};