// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#pragma once

#ifdef _WINDOWS64
#include "app/windows/Windows64_App.h"
#else
#include "app/linux/Linux_App.h"
#endif
