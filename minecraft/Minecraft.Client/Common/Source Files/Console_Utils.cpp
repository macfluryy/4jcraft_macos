#include "Minecraft.World/Header Files/stdafx.h"

#define CDECL

//--------------------------------------------------------------------------------------
// Name: DebugSpewV()
// Desc: Internal helper function
//--------------------------------------------------------------------------------------
#if !defined(_CONTENT_PACKAGE)
static void DebugSpewV(const char* strFormat, va_list pArgList) {
#if defined(__linux__)
    assert(0);
#else
    char str[2048];
    // Use the secure CRT to avoid buffer overruns. Specify a count of
    // _TRUNCATE so that too long strings will be silently truncated
    // rather than triggering an error.
    _vsnprintf_s(str, _TRUNCATE, strFormat, pArgList);
    OutputDebugStringA(str);
#endif
}
#endif

//--------------------------------------------------------------------------------------
// Name: DebugPrintf()
// Desc: Prints formatted debug spew
//--------------------------------------------------------------------------------------
void CDECL DebugPrintf(const char* strFormat, ...) {
#if !defined(_CONTENT_PACKAGE)
    va_list pArgList;
    va_start(pArgList, strFormat);
    DebugSpewV(strFormat, pArgList);
    va_end(pArgList);
#endif
}
