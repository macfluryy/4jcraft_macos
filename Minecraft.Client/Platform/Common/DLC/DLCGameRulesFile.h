#pragma once
#include "DLCGameRules.h"

class DLCGameRulesFile : public DLCGameRules
{
private:
	uint8_t *m_pbData;
	std::uint32_t m_dataBytes;

public:
	DLCGameRulesFile(const std::wstring &path);

	virtual void addData(uint8_t *pbData, std::uint32_t dataBytes);
	virtual uint8_t *getData(std::uint32_t &dataBytes);
};
