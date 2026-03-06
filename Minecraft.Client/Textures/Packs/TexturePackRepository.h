#pragma once
#include "TexturePack.h"


class Minecraft;

class TexturePackRepository 
{
public:
	static const DWORD DEFAULT_TEXTURE_PACK_ID = 0;
	static const DWORD FOLDER_TEST_TEXTURE_PACK_ID = 1;
	static const DWORD DLC_TEST_TEXTURE_PACK_ID = 2;
private:
	static TexturePack *DEFAULT_TEXTURE_PACK;
	TexturePack *m_dummyTexturePack;
	TexturePack *m_dummyDLCTexturePack;

	Minecraft *minecraft;
	File workDir;
	File multiplayerDir;
	std::vector<TexturePack *> *texturePacks;
	std::vector<TexturePack *> m_texturePacksToDelete;

	std::unordered_map<DWORD, TexturePack *> cacheById;

	TexturePack *selected;
	TexturePack *lastSelected;
    bool usingWeb;
    static const int MAX_WEB_FILESIZE = 10 * 1000 * 1000; // 10 Megabytes

public:
	TexturePackRepository(File workingDirectory, Minecraft *minecraft);
	void addDebugPacks();
private:
	void createWorkingDirecoryUnlessExists();

public:
    bool selectSkin(TexturePack *skin);

	void selectWebSkin(const std::wstring &url);

private:
	void downloadWebSkin(const std::wstring &url, File file);

public:
	bool isUsingWebSkin();
	void resetWebSkin();
    void updateList();

private:
	std::wstring getIdOrNull(File file);
    std::vector<File> getWorkDirContents();

public:
    std::vector<TexturePack *> *getAll();

	TexturePack *getSelected();
	bool shouldPromptForWebSkin();
	bool canUseWebSkin();
	bool isUsingDefaultSkin() { return selected == DEFAULT_TEXTURE_PACK; } // 4J Added
	TexturePack *getDefault() { return DEFAULT_TEXTURE_PACK; } // 4J Added

	std::vector< pair<DWORD,std::wstring> > *getTexturePackIdNames();
	bool selectTexturePackById(DWORD id); // 4J Added
	TexturePack *getTexturePackById(DWORD id); // 4J Added

	TexturePack *addTexturePackFromDLC(DLCPack *dlcPack, DWORD id);
	void clearInvalidTexturePacks();
	void updateUI();
	bool needsUIUpdate();
private:
	void removeTexturePackById(DWORD id);
public:
	unsigned int getTexturePackCount();
	TexturePack *getTexturePackByIndex(unsigned int index);
	unsigned int getTexturePackIndex(unsigned int id);
};
