#pragma once

#include "TexturePack.h"

class BufferedImage;

class AbstractTexturePack : public TexturePack {
private:
    const std::uint32_t id;
    const std::wstring name;

protected:
    File* file;
    std::wstring texname;
    std::wstring m_wsWorldName;

    std::wstring desc1;
    std::wstring desc2;

    std::uint8_t* m_iconData;
    std::uint32_t m_iconSize;

    std::uint8_t* m_comparisonData;
    std::uint32_t m_comparisonSize;

    TexturePack* fallback;

    ColourTable* m_colourTable;

protected:
    BufferedImage* iconImage;

private:
    int textureId;

protected:
    AbstractTexturePack(std::uint32_t id, File* file, const std::wstring& name,
                        TexturePack* fallback);

private:
    static std::wstring trim(std::wstring line);

protected:
    virtual void loadIcon();
    virtual void loadComparison();
    virtual void loadDescription();
    virtual void loadName();

public:
    virtual InputStream* getResource(const std::wstring& name,
                                     bool allowFallback);  // throws IOException
    // 4J Removed do to current override in TexturePack class
    // virtual InputStream *getResource(const std::wstring &name); //throws
    // IOException
    virtual DLCPack* getDLCPack() = 0;

protected:
    virtual InputStream* getResourceImplementation(
        const std::wstring& name) = 0;  // throws IOException;
public:
    virtual void unload(Textures* textures);
    virtual void load(Textures* textures);
    virtual bool hasFile(const std::wstring& name, bool allowFallback);
    virtual bool hasFile(const std::wstring& name) = 0;
    virtual std::uint32_t getId();
    virtual std::wstring getName();
    virtual std::wstring getDesc1();
    virtual std::wstring getDesc2();
    virtual std::wstring getWorldName();

    virtual std::wstring getAnimationString(const std::wstring& textureName,
                                            const std::wstring& path,
                                            bool allowFallback);

protected:
    virtual std::wstring getAnimationString(const std::wstring& textureName,
                                            const std::wstring& path);
    void loadDefaultUI();
    void loadDefaultColourTable();
    void loadDefaultHTMLColourTable();
#ifdef _XBOX
    void loadHTMLColourTableFromXuiScene(HXUIOBJ hObj);
#endif

public:
    virtual BufferedImage* getImageResource(const std::wstring& File,
                                            bool filenameHasExtension = false,
                                            bool bTitleUpdateTexture = false,
                                            const std::wstring& drive = L"");
    virtual void loadColourTable();
    virtual void loadUI();
    virtual void unloadUI();
    virtual std::wstring getXuiRootPath();
    virtual std::uint8_t* getPackIcon(std::uint32_t& imageBytes);
    virtual std::uint8_t* getPackComparison(std::uint32_t& imageBytes);
    virtual unsigned int getDLCParentPackId();
    virtual unsigned char getDLCSubPackId();
    virtual ColourTable* getColourTable() { return m_colourTable; }
    virtual ArchiveFile* getArchiveFile() { return NULL; }
};
