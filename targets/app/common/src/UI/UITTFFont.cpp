#include "UITTFFont.h"

#include <assert.h>

#include <cstdio>

#include "app/linux/Iggy/include/iggy.h"
#ifndef _ENABLEIGGY
#include "app/linux/Stubs/iggy_stubs.h"
#endif
#include "app/linux/Iggy/include/rrCore.h"
#include "app/linux/Linux_App.h"
#include "console_helpers/PortableFileIO.h"
#include "console_helpers/StringHelpers.h"

UITTFFont::UITTFFont(const std::string& name, const std::string& path,
                     S32 fallbackCharacter)
    : m_strFontName(name) {
    app.DebugPrintf("UITTFFont opening %s\n", path.c_str());
    pbData = nullptr;

    std::wstring wPath = convStringToWstring(path);
    std::FILE* file = PortableFileIO::OpenBinaryFileForRead(wPath);
    if (file == nullptr) {
        app.DebugPrintf("Failed to open TTF file\n");
        assert(false);
    }

    if (!PortableFileIO::Seek(file, 0, SEEK_END)) {
        std::fclose(file);
        app.FatalLoadError();
    }

    const int64_t endPosition = PortableFileIO::Tell(file);
    if (endPosition < 0) {
        std::fclose(file);
        app.FatalLoadError();
    }

    const std::size_t fileSize = static_cast<std::size_t>(endPosition);
    if (fileSize != 0) {
        if (!PortableFileIO::Seek(file, 0, SEEK_SET)) {
            std::fclose(file);
            app.FatalLoadError();
        }

        pbData = new std::uint8_t[fileSize];
        const std::size_t bytesRead = std::fread(pbData, 1, fileSize, file);
        const bool failed = std::ferror(file) != 0 || bytesRead != fileSize;
        std::fclose(file);
        if (failed) {
            app.FatalLoadError();
        }

        IggyFontInstallTruetypeUTF8((void*)pbData, IGGY_TTC_INDEX_none,
                                    m_strFontName.c_str(), -1,
                                    IGGY_FONTFLAG_none);

        IggyFontInstallTruetypeFallbackCodepointUTF8(
            "Mojangles_TTF", -1, IGGY_FONTFLAG_none, fallbackCharacter);

        // 4J Stu - These are so we can use the default flash controls
        IggyFontInstallTruetypeUTF8((void*)pbData, IGGY_TTC_INDEX_none,
                                    "Times New Roman", -1, IGGY_FONTFLAG_none);
        IggyFontInstallTruetypeUTF8((void*)pbData, IGGY_TTC_INDEX_none, "Arial",
                                    -1, IGGY_FONTFLAG_none);
    } else {
        std::fclose(file);
    }
}

UITTFFont::~UITTFFont() {}

std::string UITTFFont::getFontName() { return m_strFontName; }