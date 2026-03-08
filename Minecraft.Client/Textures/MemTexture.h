#pragma once
class BufferedImage;
class MemTextureProcessor;


class MemTexture {
public:
	BufferedImage *loadedImage;
    int count;
    int id;
    bool isLoaded;
	int ticksSinceLastUse;
	static const int UNUSED_TICKS_TO_FREE = 20;

    MemTexture(const std::wstring& _name, PBYTE pbData, DWORD dwBytes, MemTextureProcessor *processor);
	~MemTexture();
};