#include "../../Minecraft.World/Platform/stdafx.h"
#include "UI.h"
#include "../../Minecraft.World/Util/PortableFileIO.h"
#include "UITTFFont.h"

UITTFFont::UITTFFont(const std::string& path, S32 fallbackCharacter) {
    app.DebugPrintf("UITTFFont opening %s\n", path.c_str());
    pbData = NULL;

    std::wstring wPath = convStringToWstring(path);
    std::FILE* file = PortableFileIO::OpenBinaryFileForRead(wPath);
    if (file == NULL) {
        app.DebugPrintf("Failed to open TTF file\n");
        assert(false);
    }

    if (!PortableFileIO::Seek(file, 0, SEEK_END)) {
        std::fclose(file);
        app.FatalLoadError();
    }

    const __int64 endPosition = PortableFileIO::Tell(file);
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
                                    "Mojangles_TTF", -1, IGGY_FONTFLAG_none);

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
