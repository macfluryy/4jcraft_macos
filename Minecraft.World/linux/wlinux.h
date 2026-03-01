#ifndef WLINUX_H
#define WLINUX_H

#pragma once

#include <cstdint>
#include <cstring>

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
#define TRUE true
#define WINAPI
typedef void* HANDLE;
typedef int INT;
typedef char CHAR;
typedef void* PVOID;
typedef unsigned long* ULONG_PTR;
typedef long LONG;
typedef void VOID;
#define RtlZeroMemory(Destination,Length) memset((Destination),0,(Length))
#define ZeroMemory       RtlZeroMemory
typedef ULONGLONG PlayerUID;
typedef DWORD WORD;
#define FALSE false
typedef struct {
    DWORD LowPart;
    long long QuadPart;
    LONG HighPart;
} LARGE_INTEGER;

typedef short SHORT;

typedef struct {
    int xuidInvitee;
    int xuidInviter;
    DWORD dwTitleID;
    int hostInfo;
    bool fFromGameInvite;
} XINVITE_INFO, * PXINVITE_INFO;

typedef XINVITE_INFO INVITE_INFO;

typedef struct HXUIOBJ {
    // Stub fields representing the actual Xbox HXUIOBJ structure.
    int id;
    const char* name;
} HXUIOBJ;

typedef struct _RTL_CRITICAL_SECTION {
    // 	//
    // 	//  The following field is used for blocking when there is contention for
    // 	//  the resource
    // 	//
    //
    	union {
    		ULONG_PTR RawEvent[4];
    	} Synchronization;
    //
    // 	//
    // 	//  The following three fields control entering and exiting the critical
    // 	//  section for the resource
    // 	//
    //
    	LONG LockCount;
    	LONG RecursionCount;
    	HANDLE OwningThread;
} RTL_CRITICAL_SECTION, *PRTL_CRITICAL_SECTION;

typedef RTL_CRITICAL_SECTION CRITICAL_SECTION;

#endif // WLINUX_H