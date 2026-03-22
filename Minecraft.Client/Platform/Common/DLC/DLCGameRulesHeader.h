#pragma once

#include "DLCGameRules.h"
#include "../GameRules/LevelGenerationOptions.h"

class DLCGameRulesHeader : public DLCGameRules, public JustGrSource {
private:
    // GR-Header
    std::uint8_t* m_pbData;
    std::uint32_t m_dataBytes;

    bool m_hasData;

public:
    virtual bool requiresTexturePack() { return m_bRequiresTexturePack; }
    virtual std::uint32_t getRequiredTexturePackId() {
        return m_requiredTexturePackId;
    }
    virtual std::wstring getDefaultSaveName() { return m_defaultSaveName; }
    virtual const wchar_t* getWorldName() { return m_worldName.c_str(); }
    virtual const wchar_t* getDisplayName() { return m_displayName.c_str(); }
    virtual std::wstring getGrfPath() { return L"GameRules.grf"; }

    virtual void setRequiresTexturePack(bool x) { m_bRequiresTexturePack = x; }
    virtual void setRequiredTexturePackId(std::uint32_t x) {
        m_requiredTexturePackId = x;
    }
    virtual void setDefaultSaveName(const std::wstring& x) {
        m_defaultSaveName = x;
    }
    virtual void setWorldName(const std::wstring& x) { m_worldName = x; }
    virtual void setDisplayName(const std::wstring& x) { m_displayName = x; }
    virtual void setGrfPath(const std::wstring& x) { m_grfPath = x; }

    LevelGenerationOptions* lgo;

public:
    DLCGameRulesHeader(const std::wstring& path);

    virtual void addData(std::uint8_t* pbData, std::uint32_t dataBytes);
    virtual std::uint8_t* getData(std::uint32_t& dataBytes);

    void setGrfData(std::uint8_t* fData, std::uint32_t dataSize, StringTable*);

    virtual bool ready() { return m_hasData; }
};
