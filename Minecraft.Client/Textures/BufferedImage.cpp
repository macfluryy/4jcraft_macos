#include "../Platform/stdafx.h"
#include "../../Minecraft.World/Util/StringHelpers.h"
#include "Textures.h"
#include "../../Minecraft.World/Util/PathHelper.h"
#include "../../Minecraft.World/Util/ArrayWithLength.h"
#include "BufferedImage.h"
#if defined(__linux__)
#include <unistd.h>
#endif
#include <vector>
#include <string>


BufferedImage::BufferedImage(int width, int height, int type) {
    data[0] = new int[width * height];

    for (int i = 1; i < 10; i++) {
        data[i] = nullptr;
    }
    this->width = width;
    this->height = height;
}

void BufferedImage::ByteFlip4(unsigned int& data) {
    data = (data >> 24) | ((data >> 8) & 0x0000ff00) |
           ((data << 8) & 0x00ff0000) | (data << 24);
}
// Loads a bitmap into a buffered image - only currently supports the 2 types of
// 32-bit image that we've made so far and determines which of these is which by
// the compression method. Compression method 3 is a 32-bit image with only
// 24-bits used (ie no alpha channel) whereas method 0 is a full 32-bit image
// with a valid alpha channel.

// 4jcraft: mostly rewrote this function
BufferedImage::BufferedImage(const std::wstring& File,
                             bool filenameHasExtension,
                             bool bTitleUpdateTexture,
                             const std::wstring& drive) {
    HRESULT hr = -1;
    std::wstring filePath = File;

    for (size_t i = 0; i < filePath.length(); ++i) {
        if (filePath[i] == L'\\') filePath[i] = L'/';
    }
    for (int l = 0; l < 10; l++) data[l] = nullptr;

    std::wstring baseName = filePath;
    if (!filenameHasExtension) {
        if (baseName.size() > 4 &&
            baseName.substr(baseName.size() - 4) == L".png") {
            baseName = baseName.substr(0, baseName.size() - 4);
        }
    }

    while (!baseName.empty() && (baseName[0] == L'/' || baseName[0] == L'\\'))
        baseName = baseName.substr(1);
    if (baseName.find(L"res/") == 0) baseName = baseName.substr(4);

    std::wstring exeDir = PathHelper::GetExecutableDirW();

    for (int l = 0; l < 10; l++) {
        std::wstring mipSuffix =
            (l != 0) ? L"MipMapLevel" + _toString<int>(l + 1) : L"";
        std::wstring fileName = baseName + mipSuffix + L".png";
        std::wstring finalPath;
        bool foundOnDisk = false;

        std::vector<std::wstring> searchPaths = {
            exeDir + L"/Common/res/TitleUpdate/res/" + fileName,
            exeDir + L"/Common/res/" + fileName,
            exeDir + L"/Common/Media/Graphics/" + fileName,
            exeDir + L"/Common/Media/font/" + fileName,
            exeDir + L"/Common/res/font/" + fileName,
            exeDir + L"/Common/Media/" + fileName};

        for (auto& attempt : searchPaths) {
            size_t p;
            while ((p = attempt.find(L"//")) != std::wstring::npos)
                attempt.replace(p, 2, L"/");
            if (access(wstringtofilename(attempt), F_OK) != -1) {
                finalPath = attempt;
                foundOnDisk = true;
                break;
            }
        }

        D3DXIMAGE_INFO ImageInfo;
        ZeroMemory(&ImageInfo, sizeof(D3DXIMAGE_INFO));

        if (foundOnDisk) {
            hr = RenderManager.LoadTextureData(wstringtofilename(finalPath),
                                               &ImageInfo, &data[l]);
        } else {
            std::wstring archiveKey = L"res/" + fileName;
            if (app.hasArchiveFile(archiveKey)) {
                byteArray ba = app.getArchiveFile(archiveKey);
                hr = RenderManager.LoadTextureData(ba.data, ba.length,
                                                   &ImageInfo, &data[l]);
            }
        }

        if (hr == ERROR_SUCCESS) {
            if (l == 0) {
                width = ImageInfo.Width;
                height = ImageInfo.Height;
            }
        } else {
            if (l == 0) {
                // safety dummy to prevent crash
                width = 1;
                height = 1;
                data[0] = new int[1];
                data[0][0] = 0xFFFF00FF;
            }
            break;
        }
    }
}
BufferedImage::BufferedImage(DLCPack* dlcPack, const std::wstring& File,
                             bool filenameHasExtension) {
    HRESULT hr;
    std::wstring filePath = File;
    std::uint8_t* pbData = nullptr;
    std::uint32_t dataBytes = 0;
    for (int l = 0; l < 10; l++) data[l] = nullptr;

    for (int l = 0; l < 10; l++) {
        std::wstring name;
        std::wstring mipMapPath =
            (l != 0) ? L"MipMapLevel" + _toString<int>(l + 1) : L"";
        name = L"res" + (filenameHasExtension
                             ? filePath
                             : filePath.substr(0, filePath.length() - 4) +
                                   mipMapPath + L".png");

        if (!dlcPack->doesPackContainFile(DLCManager::e_DLCType_All, name)) {
            if (l == 0) app.FatalLoadError();
            return;
        }

        DLCFile* dlcFile = dlcPack->getFile(DLCManager::e_DLCType_All, name);
        pbData = dlcFile->getData(dataBytes);
        if (pbData == nullptr || dataBytes == 0) {
            if (l == 0) app.FatalLoadError();
            return;
        }

        D3DXIMAGE_INFO ImageInfo;
        hr = RenderManager.LoadTextureData(pbData, dataBytes, &ImageInfo,
                                           &data[l]);
        if (hr == ERROR_SUCCESS && l == 0) {
            width = ImageInfo.Width;
            height = ImageInfo.Height;
        }
    }
}

BufferedImage::BufferedImage(std::uint8_t* pbData, std::uint32_t dataBytes) {
    for (int l = 0; l < 10; l++) {
        data[l] = nullptr;
    }

    D3DXIMAGE_INFO ImageInfo;
    ZeroMemory(&ImageInfo, sizeof(D3DXIMAGE_INFO));
    HRESULT hr =
        RenderManager.LoadTextureData(pbData, dataBytes, &ImageInfo, &data[0]);

    if (hr == ERROR_SUCCESS) {
        width = ImageInfo.Width;
        height = ImageInfo.Height;
    } else {
        app.FatalLoadError();
    }
}

BufferedImage::~BufferedImage() {
    for (int i = 0; i < 10; i++) {
        delete[] data[i];
    }
}

int BufferedImage::getWidth() { return width; }

int BufferedImage::getHeight() { return height; }

void BufferedImage::getRGB(int startX, int startY, int w, int h, intArray out,
                           int offset, int scansize, int level) {
    int ww = width >> level;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            out[y * scansize + offset + x] =
                data[level][startX + x + ww * (startY + y)];
        }
    }
}

int* BufferedImage::getData() { return data[0]; }

int* BufferedImage::getData(int level) { return data[level]; }

Graphics* BufferedImage::getGraphics() { return nullptr; }

// Returns the transparency. Returns either OPAQUE, BITMASK, or TRANSLUCENT.
// Specified by:
// getTransparency in interface Transparency
// Returns:
// the transparency of this BufferedImage.
int BufferedImage::getTransparency() {
    // TODO - 4J Implement?
    return 0;
}

// Returns a subimage defined by a specified rectangular region. The returned
// BufferedImage shares the same data array as the original image. Parameters:
// x, y - the coordinates of the upper-left corner of the specified rectangular
// region w - the width of the specified rectangular region h - the height of
// the specified rectangular region Returns: a BufferedImage that is the
// subimage of this BufferedImage.
BufferedImage* BufferedImage::getSubimage(int x, int y, int w, int h) {
    // TODO - 4J Implement

    BufferedImage* img = new BufferedImage(w, h, 0);
    intArray arrayWrapper(img->data[0], w * h);
    this->getRGB(x, y, w, h, arrayWrapper, 0, w);

    int level = 1;
    // prevent overflow
    while (level < 10 && getData(level) != nullptr) {
        int ww = w >> level;
        int hh = h >> level;
        int xx = x >> level;
        int yy = y >> level;
        img->data[level] = new int[ww * hh];
        intArray levelWrapper(img->data[level], ww * hh);
        this->getRGB(xx, yy, ww, hh, levelWrapper, 0, ww, level);

        ++level;
    }

    return img;
}

void BufferedImage::preMultiplyAlpha() {
    int* curData = data[0];

    int cur = 0;
    int alpha = 0;
    int r = 0;
    int g = 0;
    int b = 0;

    int total = width * height;
    // why was it unsigned??
    for (int i = 0; i < total; ++i) {
        cur = curData[i];
        alpha = (cur >> 24) & 0xff;
        r = ((cur >> 16) & 0xff) * (float)alpha / 255;
        g = ((cur >> 8) & 0xff) * (float)alpha / 255;
        b = (cur & 0xff) * (float)alpha / 255;

        curData[i] = (r << 16) | (g << 8) | (b) | (alpha << 24);
    }
}
