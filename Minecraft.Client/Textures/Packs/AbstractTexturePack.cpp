#include "../../Platform/stdafx.h"
#include "../Textures.h"
#include "AbstractTexturePack.h"
#include "../../../Minecraft.World/IO/Streams/InputOutputStream.h"
#include "../../../Minecraft.World/Util/StringHelpers.h"

#include <limits>

AbstractTexturePack::AbstractTexturePack(std::uint32_t id, File* file,
                                         const std::wstring& name,
                                         TexturePack* fallback)
    : id(id), name(name) {
    // 4J init
    textureId = -1;
    m_colourTable = nullptr;

    this->file = file;
    this->fallback = fallback;

    m_iconData = nullptr;
    m_iconSize = 0;

    m_comparisonData = nullptr;
    m_comparisonSize = 0;

    // 4J Stu - These calls need to be in the most derived version of the class
    // loadIcon();
    // loadDescription();
}

std::wstring AbstractTexturePack::trim(std::wstring line) {
    if (!line.empty() && line.length() > 34) {
        line = line.substr(0, 34);
    }
    return line;
}

void AbstractTexturePack::loadIcon() {
}

void AbstractTexturePack::loadComparison() {
}

void AbstractTexturePack::loadDescription() {
    // 4J Unused currently
}

void AbstractTexturePack::loadName() {}

InputStream* AbstractTexturePack::getResource(
    const std::wstring& name, bool allowFallback)  // throws IOException
{
    app.DebugPrintf("texture - %ls\n", name.c_str());
    InputStream* is = getResourceImplementation(name);
    if (is == nullptr && fallback != nullptr && allowFallback) {
        is = fallback->getResource(name, true);
    }

    return is;
}

// 4J Currently removed due to override in TexturePack class
// InputStream *AbstractTexturePack::getResource(const std::wstring &name)
// //throws IOException
//{
//	return getResource(name, true);
//}

void AbstractTexturePack::unload(Textures* textures) {
    if (iconImage != nullptr && textureId != -1) {
        textures->releaseTexture(textureId);
    }
}

void AbstractTexturePack::load(Textures* textures) {
    if (iconImage != nullptr) {
        if (textureId == -1) {
            textureId = textures->getTexture(iconImage);
        }
        glBindTexture(GL_TEXTURE_2D, textureId);
        textures->clearLastBoundId();
    } else {
        // 4J Stu - Don't do this
        // textures->bindTexture(L"/gui/unknown_pack.png");
    }
}

bool AbstractTexturePack::hasFile(const std::wstring& name,
                                  bool allowFallback) {
    bool hasFile = this->hasFile(name);

    return !hasFile && (allowFallback && fallback != nullptr)
               ? fallback->hasFile(name, allowFallback)
               : hasFile;
}

std::uint32_t AbstractTexturePack::getId() { return id; }

std::wstring AbstractTexturePack::getName() { return texname; }

std::wstring AbstractTexturePack::getWorldName() { return m_wsWorldName; }

std::wstring AbstractTexturePack::getDesc1() { return desc1; }

std::wstring AbstractTexturePack::getDesc2() { return desc2; }

std::wstring AbstractTexturePack::getAnimationString(
    const std::wstring& textureName, const std::wstring& path,
    bool allowFallback) {
    return getAnimationString(textureName, path);
}

std::wstring AbstractTexturePack::getAnimationString(
    const std::wstring& textureName, const std::wstring& path) {
    std::wstring animationDefinitionFile = textureName + L".txt";

    bool requiresFallback = !hasFile(L"\\" + textureName + L".png", false);

    std::wstring result = L"";

    InputStream* fileStream =
        getResource(L"\\" + path + animationDefinitionFile, requiresFallback);

    if (fileStream) {
        // Minecraft::getInstance()->getLogger().info("Found animation info for:
        // " + animationDefinitionFile);
#if !defined(_CONTENT_PACKAGE)
        app.DebugPrintf("Found animation info for: %ls\n",
                        animationDefinitionFile.c_str());
#endif
        InputStreamReader isr(fileStream);
        BufferedReader br(&isr);

        std::wstring line = br.readLine();
        while (!line.empty()) {
            line = trimString(line);
            if (line.length() > 0) {
                result.append(L",");
                result.append(line);
            }
            line = br.readLine();
        }
        delete fileStream;
    }

    return result;
}

BufferedImage* AbstractTexturePack::getImageResource(
    const std::wstring& File, bool filenameHasExtension /*= false*/,
    bool bTitleUpdateTexture /*=false*/, const std::wstring& drive /*=L""*/) {
    const char* pchTexture = wstringtofilename(File);
    app.DebugPrintf("AbstractTexturePack::getImageResource - %s, drive is %s\n",
                    pchTexture, wstringtofilename(drive));

    return new BufferedImage(TexturePack::getResource(L"/" + File),
                             filenameHasExtension, bTitleUpdateTexture, drive);
}

void AbstractTexturePack::loadDefaultUI() {
    ui.ReloadSkin();
}

void AbstractTexturePack::loadColourTable() {
    loadDefaultColourTable();
    loadDefaultHTMLColourTable();
}

void AbstractTexturePack::loadDefaultColourTable() {
    // Load the file
    File coloursFile(
        AbstractTexturePack::getPath(true).append(L"res/colours.col"));

    if (coloursFile.exists()) {
        uint32_t dataLength = coloursFile.length();
        byteArray data(dataLength);

        FileInputStream fis(coloursFile);
        fis.read(data, 0, dataLength);
        fis.close();
        if (m_colourTable != nullptr) delete m_colourTable;
        m_colourTable = new ColourTable(data.data, dataLength);

        delete[] data.data;
    } else {
        app.DebugPrintf("Failed to load the default colours table\n");
        app.FatalLoadError();
    }
}

void AbstractTexturePack::loadDefaultHTMLColourTable() {
    if (app.hasArchiveFile(L"HTMLColours.col")) {
        byteArray textColours = app.getArchiveFile(L"HTMLColours.col");
        m_colourTable->loadColoursFromData(textColours.data,
                                           textColours.length);

        delete[] textColours.data;
    }
}


void AbstractTexturePack::loadUI() {
    loadColourTable();

}

void AbstractTexturePack::unloadUI() {
    // Do nothing
}

std::wstring AbstractTexturePack::getXuiRootPath() {
    const uintptr_t c_ModuleHandle = (uintptr_t)GetModuleHandle(nullptr);

    // Load new skin
    constexpr int LOCATOR_SIZE =
        256;  // Use this to allocate space to hold a ResourceLocator string
    wchar_t szResourceLocator[LOCATOR_SIZE];

    swprintf(szResourceLocator, LOCATOR_SIZE, L"section://%X,%ls#%ls",
             c_ModuleHandle, L"media", L"media/");
    return szResourceLocator;
}

std::uint8_t* AbstractTexturePack::getPackIcon(std::uint32_t& imageBytes) {
    if (m_iconSize == 0 || m_iconData == nullptr) loadIcon();
    imageBytes = m_iconSize;
    return m_iconData;
}

std::uint8_t* AbstractTexturePack::getPackComparison(
    std::uint32_t& imageBytes) {
    if (m_comparisonSize == 0 || m_comparisonData == nullptr) loadComparison();

    imageBytes = m_comparisonSize;
    return m_comparisonData;
}

unsigned int AbstractTexturePack::getDLCParentPackId() { return 0; }

unsigned char AbstractTexturePack::getDLCSubPackId() { return 0; }
