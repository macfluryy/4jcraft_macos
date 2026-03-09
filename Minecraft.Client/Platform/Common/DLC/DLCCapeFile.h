#pragma once
#include "DLCFile.h"

class DLCCapeFile : public DLCFile
{
public:
	DLCCapeFile(const std::wstring &path);

	virtual void addData(uint8_t *pbData, std::uint32_t dataBytes);
};
