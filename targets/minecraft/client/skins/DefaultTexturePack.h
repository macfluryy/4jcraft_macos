#pragma once
#include <string>

#include "AbstractTexturePack.h"
#include "app/mac/MacGame.h"
#include "java/InputOutputStream/InputStream.h"
#include "strings.h"

class DefaultTexturePack : public AbstractTexturePack {
public:
    DefaultTexturePack();
    DLCPack* getDLCPack() { return nullptr; }

protected:
    
    void loadIcon();
    void loadName();
    void loadDescription();

public:
    
    bool hasFile(const std::wstring& name);
    bool isTerrainUpdateCompatible();

    std::wstring getDesc1() { return app.GetString(IDS_DEFAULT_TEXTUREPACK); }

protected:
    
    InputStream* getResourceImplementation(
        const std::wstring& name);  

public:
    virtual bool hasData() { return true; }
    virtual bool hasAudio() { return false; }
    virtual bool isLoadingData() { return false; }
    virtual void loadUI();
    virtual void unloadUI();
};