#include "../../Build/stdafx.h"
#include "File.h"
#include "FileOutputStream.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cwchar>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <cstring>

//Creates a file output stream to write to the file represented by the specified File object. A new FileDescriptor object is
//created to represent this file connection.
//First, if there is a security manager, its checkWrite method is called with the path represented by the file argument as its argument.
//
//If the file exists but is a directory rather than a regular file, does not exist but cannot be created, or cannot be opened
//for any other reason then a FileNotFoundException is thrown.
//
//Parameters:
//file - the file to be opened for writing.
FileOutputStream::FileOutputStream(const File &file) : m_fileHandle( INVALID_HANDLE_VALUE )
{
	if( file.exists() && file.isDirectory())
	{
		// TODO 4J Stu - FileNotFoundException
		return;
	}

#ifdef _DURANGO
	m_fileHandle = CreateFile(
		file.getPath().c_str() , // file name
		GENERIC_WRITE, // access mode
		0, // share mode // TODO 4J Stu - Will we need to share file? Probably not but...
		NULL, // Unused
		OPEN_ALWAYS , // how to create
		FILE_ATTRIBUTE_NORMAL , // file attributes
		NULL // Unsupported
		);
#elif defined(__linux__)
	std::wstring path = file.getPath();
	char* convertedPath = new char[path.size() + 1];
	std::wcstombs(convertedPath, path.c_str(), path.size() + 1);

	m_fileHandle = (HANDLE)(intptr_t)open(convertedPath, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
	delete[] convertedPath;
#else
	m_fileHandle = CreateFile(
		wstringtofilename(file.getPath()) , // file name
		GENERIC_WRITE, // access mode
		0, // share mode // TODO 4J Stu - Will we need to share file? Probably not but...
		NULL, // Unused
		OPEN_ALWAYS , // how to create
		FILE_ATTRIBUTE_NORMAL , // file attributes
		NULL // Unsupported
		);
#endif

	if( m_fileHandle == INVALID_HANDLE_VALUE )
	{
		// TODO 4J Stu - Any form of error/exception handling
		DWORD error = GetLastError();
	}
}

FileOutputStream::~FileOutputStream()
{
	if( m_fileHandle != INVALID_HANDLE_VALUE )
		::close( (int)(intptr_t)m_fileHandle );
}

//Writes the specified byte to this file output stream. Implements the write method of OutputStream.
//Parameters:
//b - the byte to be written.
void FileOutputStream::write(unsigned int b)
{	
	byte value = (byte) b;

#if defined(_WIN32)
	BOOL result = WriteFile(
		m_fileHandle, // handle to file
		&value, // data buffer
		1, // number of bytes to write
		&numberOfBytesWritten, // number of bytes written
		NULL // overlapped buffer
		);
#else // LINUX
	int fileDescriptor = (int)(intptr_t)(m_fileHandle);
	ssize_t numberOfBytesWritten = ::write(fileDescriptor, &value, 1);
	int result = static_cast<int>(numberOfBytesWritten);
#endif // _WIN32

	if( result == 0 )
	{
		// TODO 4J Stu - Some kind of error handling
	}
	else if( numberOfBytesWritten == 0 )
	{
		// File pointer is past the end of the file
	}
}

//Writes b.length bytes from the specified byte array to this file output stream.
//Parameters:
//b - the data.
void FileOutputStream::write(byteArray b)
{
#if defined(_WIN32)
	BOOL result = WriteFile(
		m_fileHandle, // handle to file
		&b.data, // data buffer
		b.length, // number of bytes to write
		&numberOfBytesWritten, // number of bytes written
		NULL // overlapped buffer
		);
#else // Linux
	int fileDescriptor = (int)(intptr_t)(m_fileHandle);
	ssize_t numberOfBytesWritten = ::write(fileDescriptor, static_cast<const void*>(b.data), b.length);
	int result = static_cast<int>(numberOfBytesWritten);
#endif // _WIN32

	if( result == 0 )
	{
		// TODO 4J Stu - Some kind of error handling
	}
	else if( numberOfBytesWritten == 0 || numberOfBytesWritten != b.length  )
	{
		// File pointer is past the end of the file
	}
}

//Writes len bytes from the specified byte array starting at offset off to this file output stream.
//Parameters:
//b - the data.
//off - the start offset in the data.
//len - the number of bytes to write.
void FileOutputStream::write(byteArray b, unsigned int offset, unsigned int length)
{
	// 4J Stu - We don't want to write any more than the array buffer holds
	assert( length <= ( b.length - offset ) );

#if defined(_WIN32)
	DWORD numberOfBytesWritten;

	BOOL result = WriteFile(
		m_fileHandle, // handle to file
		&b[offset], // data buffer
		length, // number of bytes to write
		&numberOfBytesWritten, // number of bytes written
		NULL // overlapped buffer
		);
#else
	int fileDescriptor = (int)(intptr_t)(m_fileHandle);
	ssize_t numberOfBytesWritten = ::write(fileDescriptor, static_cast<const void*>(&b[offset]), length);
	int result = static_cast<int>(numberOfBytesWritten);
#endif // _WIN32

	if( result == 0 )
	{
		// TODO 4J Stu - Some kind of error handling
	}
	else if( numberOfBytesWritten == 0 || numberOfBytesWritten != length  )
	{
		// File pointer is past the end of the file
	}
}
//
//Closes this file output stream and releases any system resources associated with this stream.
//This file output stream may no longer be used for writing bytes.
//If this stream has an associated channel then the channel is closed as well.
void FileOutputStream::close()
{
#ifdef _WIN32
	BOOL result = CloseHandle( m_fileHandle );
#else // __linux__
	int result = ::close( (int)(intptr_t)m_fileHandle );
#endif // _WIN32
	if( result == 0 )
	{
		// TODO 4J Stu - Some kind of error handling
	}

	// Stop the dtor from trying to close it again
	m_fileHandle = INVALID_HANDLE_VALUE;
}
