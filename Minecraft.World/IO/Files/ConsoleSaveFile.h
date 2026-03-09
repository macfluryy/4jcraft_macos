#pragma once

#include "FileHeader.h"
#include "ConsoleSavePath.h"

enum class SaveFileSeekOrigin
{
	Begin,
	Current,
	End
};

class ConsoleSaveFile
{
public:
	virtual ~ConsoleSaveFile() {};

	virtual FileEntry *createFile( const ConsoleSavePath &fileName ) = 0;
	virtual void deleteFile( FileEntry *file ) = 0;
	virtual void setFilePointer( FileEntry *file, unsigned int distanceToMove, SaveFileSeekOrigin seekOrigin ) = 0;
	virtual bool writeFile(	FileEntry *file, const void *lpBuffer, unsigned int nNumberOfBytesToWrite, unsigned int *lpNumberOfBytesWritten) = 0;
	virtual bool zeroFile(FileEntry *file, unsigned int nNumberOfBytesToWrite, unsigned int *lpNumberOfBytesWritten) = 0;
	virtual bool readFile( FileEntry *file, void *lpBuffer, unsigned int nNumberOfBytesToRead, unsigned int *lpNumberOfBytesRead ) = 0;
	virtual bool closeHandle( FileEntry *file ) = 0;
	virtual void finalizeWrite() = 0;
	virtual void tick() {};

	virtual bool doesFileExist(ConsoleSavePath file) = 0;

	virtual void Flush(bool autosave, bool updateThumbnail = true) = 0;

#ifndef _CONTENT_PACKAGE
	virtual void DebugFlushToFile(void *compressedData = NULL, unsigned int compressedDataSize = 0) = 0;
#endif
	virtual unsigned int getSizeOnDisk() = 0;
	virtual std::wstring getFilename() = 0;
	virtual std::vector<FileEntry *> *getFilesWithPrefix(const std::wstring &prefix) = 0;
	virtual std::vector<FileEntry *> *getRegionFilesByDimension(unsigned int dimensionIndex) = 0;

#if defined(__PS3__) || defined(__ORBIS__) || defined(__PSVITA__)
	virtual std::wstring getPlayerDataFilenameForLoad(const PlayerUID& pUID) = 0;
	virtual std::wstring getPlayerDataFilenameForSave(const PlayerUID& pUID) = 0;
	virtual std::vector<FileEntry *> *getValidPlayerDatFiles() = 0;
#endif //__PS3__

	virtual int getSaveVersion() = 0;
	virtual int getOriginalSaveVersion() = 0;

	virtual void LockSaveAccess() = 0;
	virtual void ReleaseSaveAccess() = 0;

	virtual ESavePlatform getSavePlatform() = 0;
	virtual bool isSaveEndianDifferent() = 0;
	virtual void setLocalPlatform() = 0;
	virtual void setPlatform(ESavePlatform plat) = 0;
	virtual ByteOrder getSaveEndian() = 0;
	virtual ByteOrder getLocalEndian() = 0;
	virtual void setEndian(ByteOrder endian) = 0;

	virtual void ConvertRegionFile(File sourceFile) = 0;
	virtual void ConvertToLocalPlatform() = 0;

	virtual void *getWritePointer(FileEntry *file) { return NULL; }
};
