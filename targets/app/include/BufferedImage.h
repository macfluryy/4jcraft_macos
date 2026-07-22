#pragma once
#include <cstdint>
#include <string>
#include <vector>

class Graphics;
class DLCPack;

class BufferedImage {
private:
    int* data[10];  
    int width;
    int height;
    void ByteFlip4(unsigned int& data);  
public:
    static const int TYPE_INT_ARGB = 0;
    static const int TYPE_INT_RGB = 1;
    BufferedImage(int width, int height, int type);
    BufferedImage(const std::wstring& File, bool filenameHasExtension = false,
                  bool bTitleUpdateTexture = false,
                  const std::wstring& drive = L"");  
    BufferedImage(DLCPack* dlcPack, const std::wstring& File,
                  bool filenameHasExtension = false);              
    BufferedImage(std::uint8_t* pbData, std::uint32_t dataBytes);  
    ~BufferedImage();

    int getWidth();
    int getHeight();
    void getRGB(int startX, int startY, int w, int h, std::vector<int>& out,
                int offset, int scansize,
                int level = 0);  
    int* getData();              
    int* getData(int level);     
    Graphics* getGraphics();
    int getTransparency();
    BufferedImage* getSubimage(int x, int y, int w, int h);

    void preMultiplyAlpha();
};
