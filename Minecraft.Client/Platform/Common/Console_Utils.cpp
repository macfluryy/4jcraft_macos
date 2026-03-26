#include "../Minecraft.World/Platform/stdafx.h"

#define CDECL

//--------------------------------------------------------------------------------------
// Name: DebugSpewV()
// Desc: Internal helper function
//--------------------------------------------------------------------------------------
#ifndef _CONTENT_PACKAGE
static void DebugSpewV(const char* strFormat, va_list pArgList)
{
#if 0 || 0 || 0 || defined(__linux__)
	assert(0);
#else
	char str[2048];
	// Use the secure CRT to avoid buffer overruns. Specify a count of
	// _TRUNCATE so that too long strings will be silently truncated
	// rather than triggering an error.
	_vsnprintf_s( str, _TRUNCATE, strFormat, pArgList );
	OutputDebugStringA( str );
#endif
}
#endif

//--------------------------------------------------------------------------------------
// Name: DebugSpew()
// Desc: Prints formatted debug spew
//--------------------------------------------------------------------------------------
#ifdef  _Printf_format_string_  // VC++ 2008 and later support this annotation
void CDECL DebugSpew(_In_z_ _Printf_format_string_ const char* strFormat, ...)
#else
void CDECL DebugPrintf(const char* strFormat, ...)
#endif
{
#ifndef _CONTENT_PACKAGE
	va_list pArgList;
	va_start( pArgList, strFormat );
	DebugSpewV( strFormat, pArgList );
	va_end( pArgList );
#endif
}
