#pragma once
class BufferedImage;
class HttpTextureProcessor;

class HttpTexture {
public:
    BufferedImage* loadedImage;
    int count;
    int id;
    bool isLoaded;

    HttpTexture(const std::wstring& _url, HttpTextureProcessor* processor);
};