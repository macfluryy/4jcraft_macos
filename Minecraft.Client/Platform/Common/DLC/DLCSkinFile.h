#pragma once
#include "DLCFile.h"
#include "../../Minecraft.Client/Rendering/Models/HumanoidModel.h"

class DLCSkinFile : public DLCFile
{

private:
	std::wstring m_displayName;
	std::wstring m_themeName;
	std::wstring m_cape;
	unsigned int m_uiAnimOverrideBitmask;
	bool m_bIsFree;
	std::vector<SKIN_BOX *> m_AdditionalBoxes;

public:

	DLCSkinFile(const std::wstring &path);

	virtual void addData(uint8_t *pbData, std::uint32_t dataBytes);
	virtual void addParameter(DLCManager::EDLCParameterType type, const std::wstring &value);

	virtual std::wstring getParameterAsString(DLCManager::EDLCParameterType type);
	virtual bool getParameterAsBool(DLCManager::EDLCParameterType type);
	std::vector<SKIN_BOX *> *getAdditionalBoxes();
	int getAdditionalBoxesCount();
	unsigned int getAnimOverrideBitmask() { return m_uiAnimOverrideBitmask;}
	bool isFree() {return m_bIsFree;}
};
