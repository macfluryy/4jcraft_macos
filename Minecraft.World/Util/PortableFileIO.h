#pragma once

#include <cstddef>
#include <fstream>
#include <ios>
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

	inline BinaryReadResult ReadBinaryFile(const std::wstring &path, void *buffer, std::size_t capacity)
	{
		const std::string nativePath = wstringtofilename(path);
		std::ifstream stream(nativePath.c_str(), std::ios::binary | std::ios::ate);
		if (!stream.is_open())
		{
			return { BinaryReadStatus::not_found, 0, 0 };
		}

		const std::streamoff endPosition = stream.tellg();
		if (endPosition < 0)
		{
			return { BinaryReadStatus::read_error, 0, 0 };
		}

		const std::size_t fileSize = static_cast<std::size_t>(endPosition);
		if (fileSize > capacity)
		{
			return { BinaryReadStatus::too_large, 0, fileSize };
		}

		stream.seekg(0, std::ios::beg);
		stream.read(reinterpret_cast<char *>(buffer), static_cast<std::streamsize>(fileSize));
		const std::size_t bytesRead = static_cast<std::size_t>(stream.gcount());
		if ((!stream && !stream.eof()) || bytesRead != fileSize)
		{
			return { BinaryReadStatus::read_error, bytesRead, fileSize };
		}

		return { BinaryReadStatus::ok, bytesRead, fileSize };
	}

	inline BinaryReadResult ReadBinaryFileSegment(const std::wstring &path, std::size_t offset, void *buffer, std::size_t bytesToRead)
	{
		const std::string nativePath = wstringtofilename(path);
		std::ifstream stream(nativePath.c_str(), std::ios::binary | std::ios::ate);
		if (!stream.is_open())
		{
			return { BinaryReadStatus::not_found, 0, 0 };
		}

		const std::streamoff endPosition = stream.tellg();
		if (endPosition < 0)
		{
			return { BinaryReadStatus::read_error, 0, 0 };
		}

		const std::size_t fileSize = static_cast<std::size_t>(endPosition);
		if ((offset > fileSize) || (bytesToRead > (fileSize - offset)))
		{
			return { BinaryReadStatus::too_large, 0, fileSize };
		}

		stream.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
		stream.read(reinterpret_cast<char *>(buffer), static_cast<std::streamsize>(bytesToRead));
		const std::size_t bytesRead = static_cast<std::size_t>(stream.gcount());
		if ((!stream && !stream.eof()) || bytesRead != bytesToRead)
		{
			return { BinaryReadStatus::read_error, bytesRead, fileSize };
		}

		return { BinaryReadStatus::ok, bytesRead, fileSize };
	}
}
