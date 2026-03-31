#pragma once

#include <format>
#include <string>
#include <unordered_map>

class Texture;
class Stitcher;
class TexturePack;
class BufferedImage;

class TextureManager {
private:
    static TextureManager* instance;

    int nextID;
    typedef std::unordered_map<std::wstring, int> stringIntMap;
    typedef std::unordered_map<int, Texture*> intTextureMap;
    intTextureMap idToTextureMap;
    stringIntMap stringToIDMap;

public:
    static void createInstance();
    static TextureManager* getInstance();

private:
    TextureManager();

public:
    int createTextureID();
    Texture* getTexture(const std::wstring& name);
    void registerName(const std::wstring& name, Texture* texture);
    void registerTexture(Texture* texture);
    void unregisterTexture(const std::wstring& name, Texture* texture);
    Stitcher* createStitcher(const std::wstring& name);
    std::vector<Texture*>* createTextures(
        const std::wstring& filename, bool mipmap);  // 4J added mipmap param

private:
    std::wstring getTextureNameFromPath(const std::wstring& filename);
    bool isAnimation(const std::wstring& filename, TexturePack* texturePack);

public:
    Texture* createTexture(const std::wstring& name, int mode, int width,
                           int height, int wrap, int format, int minFilter,
                           int magFilter, bool mipmap, BufferedImage* image);
    Texture* createTexture(const std::wstring& name, int mode, int width,
                           int height, int format,
                           bool mipmap);  // 4J Added mipmap param
};