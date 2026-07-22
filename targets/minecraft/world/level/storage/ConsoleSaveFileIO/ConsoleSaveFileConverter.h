#pragma once

#include "java/File.h"

class FileEntry;
class ConsoleSaveFile;
class ProgressRenderer;
class ProgressListener;



class ConsoleSaveFileConverter {
private:
    static void ProcessSimpleFile(ConsoleSaveFile* sourceSave,
                                  FileEntry* sourceFileEntry,
                                  ConsoleSaveFile* targetSave,
                                  FileEntry* targetFileEntry);
    static void ProcessStandardRegionFile(ConsoleSaveFile* sourceSave,
                                          File sourceFile,
                                          ConsoleSaveFile* targetSave,
                                          File targetFile);

public:
    static void ConvertSave(ConsoleSaveFile* sourceSave,
                            ConsoleSaveFile* targetSave,
                            ProgressListener* progress);
};