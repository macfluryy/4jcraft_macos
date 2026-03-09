#pragma once

#include <cstddef>
#include <cstdio>
#include <string>

#include "StringHelpers.h"

namespace PortableFileIO
{
	enum class BinaryReadStatus
	{
		ok,
		not_found,
		too_large,
		read_error,
	};

	struct BinaryReadResult
	{
		BinaryReadStatus status;
		std::size_t bytesRead;
		std::size_t fileSize;
	};

	inline std::FILE *OpenBinaryFileForRead(const std::wstring &path)
	{
#if defined(_WIN32)
		return _wfopen(path.c_str(), L"rb");
#else
		const std::string nativePath = wstringtofilename(path);
		return std::fopen(nativePath.c_str(), "rb");
#endif
	}

	inline bool Seek(std::FILE *file, std::size_t offset, int origin)
	{
#if defined(_WIN32)
		return _fseeki64(file, static_cast<__int64>(offset), origin) == 0;
#else
		return fseeko(file, static_cast<off_t>(offset), origin) == 0;
#endif
	}

	inline __int64 Tell(std::FILE *file)
	{
#if defined(_WIN32)
		return _ftelli64(file);
#else
		return static_cast<__int64>(ftello(file));
#endif
	}

	inline BinaryReadResult ReadBinaryFile(const std::wstring &path, void *buffer, std::size_t capacity)
	{
		std::FILE *stream = OpenBinaryFileForRead(path);
		if (stream == NULL)
		{
			return { BinaryReadStatus::not_found, 0, 0 };
		}

		if (!Seek(stream, 0, SEEK_END))
		{
			std::fclose(stream);
			return { BinaryReadStatus::read_error, 0, 0 };
		}

		const __int64 endPosition = Tell(stream);
		if (endPosition < 0)
		{
			std::fclose(stream);
			return { BinaryReadStatus::read_error, 0, 0 };
		}

		const std::size_t fileSize = static_cast<std::size_t>(endPosition);
		if (fileSize > capacity)
		{
			std::fclose(stream);
			return { BinaryReadStatus::too_large, 0, fileSize };
		}

		if (!Seek(stream, 0, SEEK_SET))
		{
			std::fclose(stream);
			return { BinaryReadStatus::read_error, 0, fileSize };
		}

		const std::size_t bytesRead = std::fread(buffer, 1, fileSize, stream);
		const bool failed = std::ferror(stream) != 0;
		std::fclose(stream);
		if (failed || bytesRead != fileSize)
		{
			return { BinaryReadStatus::read_error, bytesRead, fileSize };
		}

		return { BinaryReadStatus::ok, bytesRead, fileSize };
	}

	inline BinaryReadResult ReadBinaryFileSegment(const std::wstring &path, std::size_t offset, void *buffer, std::size_t bytesToRead)
	{
		std::FILE *stream = OpenBinaryFileForRead(path);
		if (stream == NULL)
		{
			return { BinaryReadStatus::not_found, 0, 0 };
		}

		if (!Seek(stream, 0, SEEK_END))
		{
			std::fclose(stream);
			return { BinaryReadStatus::read_error, 0, 0 };
		}

		const __int64 endPosition = Tell(stream);
		if (endPosition < 0)
		{
			std::fclose(stream);
			return { BinaryReadStatus::read_error, 0, 0 };
		}

		const std::size_t fileSize = static_cast<std::size_t>(endPosition);
		if ((offset > fileSize) || (bytesToRead > (fileSize - offset)))
		{
			std::fclose(stream);
			return { BinaryReadStatus::too_large, 0, fileSize };
		}

		if (!Seek(stream, offset, SEEK_SET))
		{
			std::fclose(stream);
			return { BinaryReadStatus::read_error, 0, fileSize };
		}

		const std::size_t bytesRead = std::fread(buffer, 1, bytesToRead, stream);
		const bool failed = std::ferror(stream) != 0;
		std::fclose(stream);
		if (failed || bytesRead != bytesToRead)
		{
			return { BinaryReadStatus::read_error, bytesRead, fileSize };
		}

		return { BinaryReadStatus::ok, bytesRead, fileSize };
	}
}
