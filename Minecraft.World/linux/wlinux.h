#ifndef WLINUX_H
#define WLINUX_H

#pragma once

#include <cstdint>
#include <cstring>
#include <string>

#define TRUE true
#define FALSE false
#define RtlZeroMemory(Destination,Length) memset((Destination),0,(Length))
#define ZeroMemory       RtlZeroMemory
#define WINAPI

typedef unsigned int DWORD;
typedef const char *LPCSTR;
typedef bool BOOL;
typedef void* LPVOID;
typedef char WCHAR;
typedef unsigned char BYTE;
typedef BYTE* PBYTE;
typedef const wchar_t* LPCWSTR;
typedef unsigned long long ULONGLONG;
typedef int HRESULT;
typedef unsigned int UINT;
typedef void* HANDLE;
typedef int INT;
typedef char CHAR;
typedef void* PVOID;
typedef unsigned long* ULONG_PTR;
typedef long LONG;
typedef void VOID;
typedef ULONGLONG PlayerUID;
typedef DWORD WORD;
typedef struct {
    DWORD LowPart;
    long long QuadPart;
    LONG HighPart;
} LARGE_INTEGER;
typedef long long LONGLONG;
typedef size_t SIZE_T;
typedef std::wstring LPWSTR;

// https://learn.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-filetime
typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;

typedef short SHORT;

typedef VOID* XMEMCOMPRESSION_CONTEXT;
typedef VOID* XMEMDECOMPRESSION_CONTEXT;

typedef float FLOAT;

#include "../x64headers/extraX64.h"

#endif // WLINUX_H
