#pragma once
#include <cstdint>
#include <format>
#include <string>
#include <vector>

#include "DLCFile.h"
#include "Minecraft.Client/Common/src/DLC/DLCManager.h"
#include "Minecraft.Client/include/SkinBox.h"
#include "minecraft/client/model/HumanoidModel.h"

class DLCSkinFile : public DLCFile {
private:
    std::wstring m_displayName;
    std::wstring m_themeName;
    std::wstring m_cape;
    unsigned int m_uiAnimOverrideBitmask;
    bool m_bIsFree;
    std::vector<SKIN_BOX*> m_AdditionalBoxes;

public:
    DLCSkinFile(const std::wstring& path);

    virtual void addData(std::uint8_t* pbData, std::uint32_t dataBytes);
    virtual void addParameter(DLCManager::EDLCParameterType type,
                              const std::wstring& value);

    virtual std::wstring getParameterAsString(
        DLCManager::EDLCParameterType type);
    virtual bool getParameterAsBool(DLCManager::EDLCParameterType type);
    std::vector<SKIN_BOX*>* getAdditionalBoxes();
    int getAdditionalBoxesCount();
    unsigned int getAnimOverrideBitmask() { return m_uiAnimOverrideBitmask; }
    bool isFree() { return m_bIsFree; }
};
