#pragma once

#include "DirectoryLevelStorageSource.h"
#include "../../IO/Files/FileFilter.h"
#include "../../IO/Files/FilenameFilter.h"

class ProgressListener;
class LevelStorage;

class McRegionLevelStorageSource : public DirectoryLevelStorageSource {
public:
    class ChunkFile;

    McRegionLevelStorageSource(File dir);
    virtual std::wstring getName();
    virtual std::vector<LevelSummary*>* getLevelList();
    virtual void clearAll();
    virtual std::shared_ptr<LevelStorage> selectLevel(
        ConsoleSaveFile* saveFile, const std::wstring& levelId,
        bool createPlayerDir);
    virtual bool isConvertible(ConsoleSaveFile* saveFile,
                               const std::wstring& levelId);
    virtual bool requiresConversion(ConsoleSaveFile* saveFile,
                                    const std::wstring& levelId);
    virtual bool convertLevel(ConsoleSaveFile* saveFile,
                              const std::wstring& levelId,
                              ProgressListener* progress);

private:
#if 0
	// 4J - not required anymore
	void addRegions(File &baseFolder, std::vector<ChunkFile *> *dest, std::vector<File *> *firstLevelFolders);
#endif
    void convertRegions(File& baseFolder, std::vector<ChunkFile*>* chunkFiles,
                        int currentCount, int totalCount,
                        ProgressListener* progress);
    void eraseFolders(std::vector<File*>* folders, int currentCount,
                      int totalCount, ProgressListener* progress);

public:
#if 0
	// 4J - not required anymore
    static class FolderFilter : public FileFilter 
	{
	public:
		static const std::tr1::wregex chunkFolderPattern; // was Pattern
		bool accept(File *file);
    };

    static class ChunkFilter : public FilenameFilter 
	{
	public:
		static const std::tr1::wregex chunkFilePattern; // was Pattern
        bool accept(File *dir, const std::wstring& name);
    };

    static class ChunkFile // implements Comparable<ChunkFile>
	{
	private:
		/* const */ File *file;
		/* const */ int x;
		/* const */ int z;

	public:
		ChunkFile(File *file);
		int compareTo(ChunkFile *rhs);
        File *getFile();
        int getX();
        int getZ();

		// a < b
		bool operator<( ChunkFile *b );
    };
#endif
};
