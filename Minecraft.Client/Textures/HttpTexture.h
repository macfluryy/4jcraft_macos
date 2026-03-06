#pragma once
class BufferedImage;
class HttpTextureProcessor;
using namespace std;

class HttpTexture {
public:
	BufferedImage *loadedImage;
    int count;
    int id;
    bool isLoaded;

    HttpTexture(const std::wstring& _url, HttpTextureProcessor *processor);
};