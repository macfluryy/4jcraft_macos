#pragma once

#include <cstdint>

#include "../../Platform/Common/App_enums.h"

class InputStream;
class Minecraft;
class ArchiveFile;

class TexturePack
{
public:

	TexturePack() { m_bHasAudio=false;}
	virtual bool hasData() = 0;
	virtual bool hasAudio() { return m_bHasAudio;}
	virtual void setHasAudio(bool bVal) {m_bHasAudio=bVal;}	
	virtual bool isLoadingData() = 0;
	virtual void loadData() {}
	virtual void unload(Textures *textures) = 0;
    virtual void load(Textures *textures) = 0;
    virtual InputStream *getResource(const std::wstring &name, bool allowFallback) = 0;// throws IOException;
    //virtual InputStream *getResource(const std::wstring &name) = 0;// throws IOException;
    virtual std::uint32_t getId() = 0;
    virtual std::wstring getName() = 0;
    virtual std::wstring getDesc1() = 0;
    virtual std::wstring getDesc2() = 0;
    virtual bool hasFile(const std::wstring &name, bool allowFallback) = 0;
    virtual bool isTerrainUpdateCompatible() = 0;

    virtual std::wstring getResource(const std::wstring& name)	// 4J - changed to just return a name rather than an input stream
	{
		/* 4J - TODO
        return TexturePack.class.getResourceAsStream(name);
		*/
		return name;
    }
	virtual DLCPack * getDLCPack() { return NULL;}


	// 4J Added
	virtual std::wstring getPath(bool bTitleUpdateTexture = false);
	virtual std::wstring getAnimationString(const std::wstring &textureName, const std::wstring &path, bool allowFallback) = 0;
	virtual BufferedImage *getImageResource(const std::wstring& File, bool filenameHasExtension = false, bool bTitleUpdateTexture=false, const std::wstring &drive =L"") = 0;
	virtual void loadColourTable() = 0;
	virtual void loadUI() = 0;
	virtual void unloadUI() = 0;
	virtual std::wstring getXuiRootPath() = 0;
	virtual std::uint8_t *getPackIcon(std::uint32_t &imageBytes) = 0;
	virtual std::uint8_t *getPackComparison(std::uint32_t &imageBytes) = 0;
	virtual unsigned int getDLCParentPackId() = 0;
	virtual unsigned char getDLCSubPackId() = 0;
	virtual ColourTable *getColourTable() = 0;
	virtual ArchiveFile *getArchiveFile() = 0;

private:
	bool m_bHasAudio;

};
